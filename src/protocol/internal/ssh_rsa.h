#ifndef SSH_RSA
#define SSH_RSA

#include "ssh_session.h"
#include "ssh_error.h"
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* } */

int verifyRSAKey(ssh_session *ss, const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
