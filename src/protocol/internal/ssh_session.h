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

	const char *V_C, *V_S;
	size_t I_C_len, I_S_len, K_S_len;
	char *I_C, *I_S;
	uint8_t *K_S;
} ssh_session;

void computeKey(ssh_session *, int, char, unsigned char []);

#ifdef __cplusplus
}
#endif

#endif
