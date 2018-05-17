#include "ssh_cipher.h"
#include <openssl/des.h>
#include <string.h>

struct ssh1_3des_priv
{
	DES_cblock d_IV1;
	DES_cblock d_IV2;
	DES_cblock d_IV3;
	DES_key_schedule d_key1;
	DES_key_schedule d_key2;
	DES_key_schedule d_key3;
};

static int
init_3des(SSH_CIPHER* my, const uint8_t *dkey, const uint8_t *IV)
{
	struct ssh1_3des_priv *priv = (struct ssh1_3des_priv*)my->priv;
	const_DES_cblock *key = (const_DES_cblock*)dkey;
	DES_set_key(key, &priv->d_key1);
	DES_set_key(key+1, &priv->d_key2);
	DES_set_key(key+2, &priv->d_key3);
	memset(priv->d_IV1, 0, sizeof(DES_cblock));
	memset(priv->d_IV2, 0, sizeof(DES_cblock));
	memset(priv->d_IV3, 0, sizeof(DES_cblock));
	my->started = true;

	return 1;
}

static void
cleanup(SSH_CIPHER* my)
{
	if (my->priv!=NULL)
		free(my->priv);

	free(my);
}

static int
decrypt(SSH_CIPHER* my, const uint8_t *source, uint8_t *dest, size_t len)
{
	struct ssh1_3des_priv *priv = (struct ssh1_3des_priv*)my->priv;
	DES_ncbc_encrypt(source, dest, len, &priv->d_key3, &priv->d_IV3, 0);
	DES_ncbc_encrypt(dest, dest, len, &priv->d_key2, &priv->d_IV2, 1);
	DES_ncbc_encrypt(dest, dest, len, &priv->d_key1, &priv->d_IV1, 0);
	return 1;
}

static int
encrypt(SSH_CIPHER* my, const uint8_t *source, uint8_t *dest, size_t len)
{
	struct ssh1_3des_priv *priv = (struct ssh1_3des_priv*)my->priv;
	DES_ncbc_encrypt(source, dest, len, &priv->d_key1, &priv->d_IV1, 1);
	DES_ncbc_encrypt(dest, dest, len, &priv->d_key2, &priv->d_IV2, 0);
	DES_ncbc_encrypt(dest, dest, len, &priv->d_key3, &priv->d_IV3, 1);
	return 1;
}


SSH_CIPHER*
new_3des_ssh1(int enc)
{
	SSH_CIPHER *cipher = (SSH_CIPHER*)malloc(sizeof(SSH_CIPHER));
	cipher->priv = malloc(sizeof(struct ssh1_3des_priv));
	cipher->blkSize = 8;
	cipher->IVSize = 0;
	cipher->keySize = 24;
	if (enc)
		cipher->crypt = encrypt;
	else
		cipher->crypt = decrypt;

	cipher->init = init_3des;
	cipher->cleanup = cleanup;
	cipher->started = false;

	return cipher;
}
