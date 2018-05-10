#include <string.h>
#include "ssh_pubkey_crypto.h"
#include "ssh_rsa.h"
#include "buffer.h"

static RSA *CreateRSAContext(unsigned char *hostkey, int len)
{
	int algo_len, e_len, n_len;
	RSA *rsa = RSA_new();
	BIGNUM *rsa_e = BN_new();
	BIGNUM *rsa_n = BN_new();

	if (len >= 4)
		algo_len = be32toh(*(uint32_t *)hostkey);
	else
		goto fail;
	hostkey += 4;
	len -= 4;

	if (!(len >= 7 && algo_len == 7 && memcmp(hostkey, "ssh-rsa", 7) == 0))
		goto fail;
	hostkey += 7;
	len -= 7;

	if (len >= 4)
		e_len = be32toh(*(uint32_t *)hostkey);
	else
		goto fail;
	if (len >= 4 + e_len)
		BN_mpi2bn(hostkey, 4 + e_len, rsa_e);
	else
		goto fail;
	hostkey += 4 + e_len;
	len -= 4 + e_len;

	if (len >= 4)
		n_len = be32toh(*(uint32_t *)hostkey);
	else
		goto fail;
	if (len >= 4 + n_len)
		BN_mpi2bn(hostkey, 4 + n_len, rsa_n);
	else
		goto fail;
	hostkey += 4 + n_len;
	len -= 4 + n_len;

#ifdef HAVE_OPAQUE_STRUCTS
	RSA_set0_key(rsa, rsa_n, rsa_e, NULL);
#else
	rsa->n = rsa_n;
	rsa->e = rsa_e;
#endif

	return rsa;
fail:
	BN_clear_free(rsa_e);
	BN_clear_free(rsa_n);
	RSA_free(rsa);
	return NULL;
}

static inline const uint8_t *get_str(const uint8_t **data, uint32_t *L)
{
	const uint8_t *ptr = *data;
	uint32_t len = be32toh(*(uint32_t *)ptr);
	*data = ptr + 4 + len;
	*L = len;
	return ptr + 4;
}

int verifyRSAKey(ssh_session *ss, const uint8_t *data, size_t len)
{
	const uint8_t *next = data;

	uint32_t K_S_len;
	const uint8_t *K_S;
	K_S = get_str(&next, &K_S_len);
	ss->K_S_len = K_S_len;
	ss->K_S = (uint8_t *)malloc(K_S_len);
	memcpy(ss->K_S, K_S, K_S_len);

	uint32_t mpint_f_len;
	const uint8_t *mpint_f;
	mpint_f = get_str(&next, &mpint_f_len);

	if (ssh_dh_compute_secret(ss->dh, mpint_f, mpint_f_len) < 0)
		return -ESECRET;

	uint32_t s_len;
	const uint8_t *s;
	s = get_str(&next, &s_len);

	buffer vbuf;
	buffer_init(&vbuf);
	buffer_append_string(&vbuf, ss->V_C, strlen(ss->V_C));
	buffer_append_string(&vbuf, ss->V_S, strlen(ss->V_S));
	buffer_append_string(&vbuf, ss->I_C, ss->I_C_len);
	buffer_append_string(&vbuf, ss->I_S, ss->I_S_len);
	buffer_append_string(&vbuf, (const char *)K_S, K_S_len);
	buffer_append(&vbuf, ss->dh->mpint_e, ss->dh->e_len);
	buffer_append_string(&vbuf, (const char *)mpint_f, mpint_f_len);
	buffer_append(&vbuf, ss->dh->secret, ss->dh->secret_len);

	ssh_dh_hash(ss->dh, buffer_data(&vbuf), ss->H, buffer_len(&vbuf));

	buffer_deinit(&vbuf);

	// Start verify
	// ssh-rsa specifies SHA-1 hashing
	unsigned char s_H[SHA_DIGEST_LENGTH];
	SHA1(ss->H, ss->dh->digest.hashlen, s_H);

	// Ignore the first 15 bytes of the signature of H sent from server:
	// algorithm_name_length[4], algorithm_name[7]("ssh-rsa") and signature_length[4].
	RSA *rsactx = CreateRSAContext((unsigned char *)K_S, K_S_len);
	if (rsactx == NULL)
		return -ERSA;

	int sig_len = s_len - 15;
	const uint8_t *sig = s + 15;
	int res = RSA_verify(NID_sha1, s_H, SHA_DIGEST_LENGTH, sig, sig_len,
			     rsactx);

	RSA_free(rsactx);

	return (res == 1);
}
