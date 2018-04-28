/*
 * ssh_mac.h: the MAC algorithms used in SSH
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

#ifndef SSH_MAC_H
#define SSH_MAC_H

#include <stdlib.h>
#include <stdint.h>
#include <openssl/evp.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct ssh_mac_t SSH_MAC;
	typedef SSH_MAC* (*create_t)(const SSH_MAC*);
	typedef void (*mac_cleanup_t)(SSH_MAC*);
	typedef void (*mac_t)(SSH_MAC*, const unsigned char*, int, unsigned char*);

	struct ssh_mac_t
	{
		const char *name;
		void *priv;
		create_t new_mac;
		mac_cleanup_t cleanup;
		mac_t getmac;
		unsigned char key[32];
		size_t dgstSize;
		size_t keySize;
	};

	const struct ssh_mac_t *search_mac(const char *s);
	extern const char all_macs_list[];

#ifdef __cplusplus
}
#endif

#endif
