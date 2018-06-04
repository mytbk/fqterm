#ifndef CRYPTO_SCALARMULT_H
#define CRYPTO_SCALARMULT_H

typedef unsigned char u8;

#ifdef __cplusplus
extern "C" {
#endif

int crypto_scalarmult_base(unsigned char *q,const unsigned char *n);
int crypto_scalarmult(u8 *mypublic, const u8 *secret, const u8 *basepoint);

#ifdef __cplusplus
}
#endif

#endif
