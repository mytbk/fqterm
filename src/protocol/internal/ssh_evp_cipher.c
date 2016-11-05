#include "ssh_cipher.h"

struct evp_priv
{
	SSH_EVP evp;
	EVP_CIPHER_CTX *ctx;
	int enc;
};

static int
cipher_init(SSH_CIPHER* my)
{
	struct evp_priv *priv = (struct evp_priv*)my->priv;
	priv->ctx = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_init(priv->ctx);
	return EVP_CipherInit(priv->ctx, priv->evp(), my->key, my->IV, priv->enc);
}

static int
do_crypt(SSH_CIPHER* my, const uint8_t* in, uint8_t* out, size_t l)
{
	return EVP_Cipher(((struct evp_priv*)my->priv)->ctx, out, in, l);
}

static void
cleanup(SSH_CIPHER* my)
{
	if (my->IV!=NULL)
		free(my->IV);

	if (my->key!=NULL)
		free(my->key);

	if (my->priv!=NULL) {
		struct evp_priv *priv = my->priv;
		if (priv->ctx!=NULL)
			EVP_CIPHER_CTX_free(priv->ctx);

		free(priv);
	}

	free(my);
}

SSH_CIPHER*
new_ssh_cipher_evp(SSH_EVP evp, size_t ks, size_t is, size_t bs, int enc)
{
	SSH_CIPHER *cipher = (SSH_CIPHER*)malloc(sizeof(SSH_CIPHER));
	cipher->priv = malloc(sizeof(struct evp_priv));
	struct evp_priv *priv = (struct evp_priv*)cipher->priv;
	priv->evp = evp;
	priv->enc = enc;
	priv->ctx = NULL;
	cipher->blkSize = bs;
	cipher->keySize = ks;
	cipher->IVSize = is;
	cipher->key = (unsigned char*)malloc(ks);
	cipher->IV = (unsigned char*)malloc(is);
	cipher->init = cipher_init;
	cipher->crypt = do_crypt;
	cipher->cleanup = cleanup;
	return cipher;
}
