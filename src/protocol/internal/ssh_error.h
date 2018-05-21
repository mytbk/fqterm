#ifndef SSH_ERROR_H
#define SSH_ERROR_H

enum {
	ESECRET = 1, /* error comupting DH secret */
	ERSA, /* RSA error */
	ECRYPT, /* encrypt/decrypt error */
	ETOOSMALL, /* buffer too small */
	ETOOBIG, /* buffer too big */
	ECRC32, /* CRC32 error */
	EMAC, /* MAC error */
};

#endif
