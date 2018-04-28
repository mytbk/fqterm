#ifndef SSH_CIPHER_H
#define SSH_CIPHER_H

#include <stdlib.h>
#include <stdint.h>
#include <openssl/evp.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct ssh_cipher_t SSH_CIPHER;
	typedef int (*crypt_t)(SSH_CIPHER*, const uint8_t*, uint8_t*, size_t);
	typedef int (*init_t)(SSH_CIPHER*);
	typedef void (*cleanup_t)(SSH_CIPHER*);

	struct ssh_cipher_t
	{
		/*
		 * priv is used for things like EVP_CIPHER_CTX and EVP_CIPHER
		 *
		 * We use only one crypt function for encrypt or decrypt.
		 *
		 * Before using the crypto function, IV and key must
		 * be set and then init function must be called
		 */
		const char *name;
		unsigned char *IV;
		unsigned char *key;
		void *priv;
		crypt_t crypt;
		init_t init;
		cleanup_t cleanup;
		size_t blkSize;
		size_t keySize;
		size_t IVSize;
	};

	typedef const EVP_CIPHER*(*SSH_EVP)(void);
	typedef SSH_CIPHER*(*NEW_CIPHER)(int);

	SSH_CIPHER* new_ssh_cipher_evp(SSH_EVP, size_t key, size_t iv, size_t blk, int enc);
	SSH_CIPHER* new_3des_ssh1(int);
	/* all_ciphers.c */
	extern const char all_ciphers_list[];
	NEW_CIPHER search_cipher(const char *s);

#ifdef __cplusplus
}
#endif

#endif
