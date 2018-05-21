/*
 * ssh_packet.h: SSH packet maker
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

#ifndef SSH_PACKET_H
#define SSH_PACKET_H

#include "buffer.h"
#include "ssh_cipher.h"
#include "ssh_mac.h"

#ifdef __cplusplus
extern "C" {
#endif /* } */

void make_ssh1_packet(buffer *src, buffer *dest, SSH_CIPHER *);
int make_ssh2_packet(buffer *src, buffer *dest, SSH_CIPHER *,
		SSH_MAC *, bool is_mac_, uint32_t *seq);
/* parse_ssh{1,2}_packet: return the length of the received data */
int parse_ssh1_packet(buffer *input, buffer *output, SSH_CIPHER *cipher);
int parse_ssh2_packet(buffer *input, buffer *recvbuf, SSH_CIPHER *cipher,
		SSH_MAC *mac, bool is_mac, uint32_t *decrypted, uint32_t *seq);

#ifdef __cplusplus
}
#endif

#endif
