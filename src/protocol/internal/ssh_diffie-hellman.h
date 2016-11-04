#ifndef SSH_DH_H
#define SSH_DH_H

#include <openssl/bn.h>
#include <openssl/sha.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef unsigned char* (*hash_t)(const unsigned char *, size_t, unsigned char *);

	typedef struct ssh_diffie_hellman
	{
		BIGNUM *g; // generator
		BIGNUM *p; // prime
		hash_t hash; // can be SHA1 or SHA256
		size_t hashlen;
	} SSH_DH;

	void ssh_dh_free(SSH_DH*);
	SSH_DH *ssh_dh_group1_sha1(void);
	SSH_DH *ssh_dh_group14_sha1(void);

	typedef SSH_DH*(*NEW_DH)(void);
	extern const char all_dh_list[];
	NEW_DH search_dh(const char *s);

#ifdef __cplusplus
}
#endif

#endif
