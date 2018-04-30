#ifndef SSH_SESSION_H
#define SSH_SESSION_H

#include "ssh_diffie-hellman.h"

#ifdef __cplusplus
extern "C" {
#endif /* } for better indentation in Vim */

typedef struct
{
	unsigned char *session_id;
	unsigned char H[SHA512_DIGEST_LENGTH];
	SSH_DH *dh;
} ssh_session;

void computeKey(ssh_session *, int, char, unsigned char []);

#ifdef __cplusplus
}
#endif

#endif
