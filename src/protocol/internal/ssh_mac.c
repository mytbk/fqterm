/*
 * ssh_mac.c: the MAC algorithms used in SSH
 * Copyright (C) 2018  Iru Cai <mytbk920423@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "ssh_mac.h"
#include "ssh_crypto_common.h"
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

static struct ssh_mac_t *new_evp_mac(const struct ssh_mac_t *t)
{
	struct ssh_mac_t *m = (struct ssh_mac_t *)malloc(sizeof(struct ssh_mac_t));
	const EVP_MD *evp_md = ((const EVP_MD *(*)(void))(t->priv))();
	*m = *t;
	m->priv = evp_md;
	return m;
}

static void free_evp_mac(struct ssh_mac_t *m)
{
	free(m);
}

static void get_evp_digest(SSH_MAC *my, const unsigned char *d, int len, unsigned char *dgst)
{
	unsigned int tmp;
	HMAC((const EVP_MD *)(my->priv), my->key, my->keySize, d, len, dgst, &tmp);
}

/* RFC 6668 */
struct ssh_mac_t hmac_sha2_256 = {
	.name = "hmac-sha2-256",
	.priv = (const void*)EVP_sha256,
	.new_mac = new_evp_mac,
	.cleanup = free_evp_mac,
	.getmac = get_evp_digest,
	.dgstSize = 32,
	.keySize = 32
};

struct ssh_mac_t hmac_sha1 = {
	.name = "hmac-sha1",
	.priv = (const void*)EVP_sha1,
	.new_mac = new_evp_mac,
	.cleanup = free_evp_mac,
	.getmac = get_evp_digest,
	.dgstSize = 20,
	.keySize = 20
};

static name_list all_macs = {
	{ "hmac-sha2-256", &hmac_sha2_256 },
	{ "hmac-sha1", &hmac_sha1 },
	{ NULL, NULL }
};

const struct ssh_mac_t * search_mac(const char *s)
{
	int i = search_name(all_macs, s);
	if (i!=-1)
		return all_macs[i].f;
	else
		return NULL;
}

const char all_macs_list[] = "hmac-sha2-256,hmac-sha1";
