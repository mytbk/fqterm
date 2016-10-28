/* This file is part of FQTerm project
 * written by Iru Cai <mytbk920423@gmail.com>
 */

#ifndef SSH_PUBKEY_CRYPTO_H
#define SSH_PUBKEY_CRYPTO_H

#include <openssl/rsa.h>

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(LIBRESSL_VERSION_NUMBER)
# define HAVE_OPAQUE_STRUCTS 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

	enum pubkey_type {
		SSH_RSA
	};

	struct ssh_pubkey_t
	{
		enum pubkey_type key_type; /* now only RSA is supported */
		union {
			RSA *ssh_rsa;
		} key;
	};

	struct ssh_pubkey_t *ssh_pubkey_new(enum pubkey_type);
	void ssh_pubkey_free(struct ssh_pubkey_t*);
	int ssh_pubkey_encrypt(struct ssh_pubkey_t *k, BIGNUM *out, BIGNUM *in);

#ifdef HAVE_OPAQUE_STRUCTS
#define ssh_pubkey_setrsa(k,n,e,d) RSA_set0_key(k->key.ssh_rsa, n, e, d)
#define ssh_pubkey_getrsa(k,n,e,d) RSA_get0_key(k->key.ssh_rsa, n, e, d)
#else
	int ssh_pubkey_setrsa(struct ssh_pubkey_t *k, BIGNUM *n, BIGNUM *e, BIGNUM *d);
	int ssh_pubkey_getrsa(struct ssh_pubkey_t *k, const BIGNUM **n, const BIGNUM **e, const BIGNUM **d);
#endif

#ifdef __cplusplus
}
#endif

#endif
