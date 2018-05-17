/*
 * ssh_packet.c: SSH packet maker
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

#include "ssh_packet.h"
#include "crc32.h"
#include "ssh_error.h"

//==============================================================================
//
// SSH1 Packet Structure:
//  --------------------------------------------------------------------------
//  | length | padding  | type  |                data                | crc32 |
//  --------------------------------------------------------------------------
//  | uint32 | 1-7bytes | uchar |                                    | 4bytes|
//  --------------------------------------------------------------------------
//  encrypt = padding + type + data + crc32
//  length =  type + data + crc32
//
//==============================================================================
void make_ssh1_packet(buffer *orig_data, buffer *data_to_send,
		      SSH_CIPHER *cipher)
{
	int len, padding_len;
	uint32_t padding[2];
	uint8_t *data = buffer_data(orig_data);
	size_t data_len = buffer_len(orig_data);

	len = data_len + 4; //CRC32
	padding_len = 8 - (len % 8);
	padding[0] = rand();
	padding[1] = rand();

	buffer_clear(data_to_send);
	buffer_append_be32(data_to_send, len);
	buffer_append(data_to_send, (const uint8_t *)padding, padding_len);
	buffer_append(data_to_send, data, data_len);
	buffer_append_be32(data_to_send,
			   ssh_crc32(buffer_data(data_to_send) + 4,
				     buffer_len(data_to_send) - 4));

	if (cipher->started) {
		cipher->crypt(cipher, buffer_data(data_to_send) + 4,
			      buffer_data(data_to_send) + 4,
			      buffer_len(data_to_send) - 4);
	}
}

//==============================================================================
// SSH2 Packet Structure:
//      uint32    packet_length
//      byte      padding_length
//      byte[n1]  payload; n1 = packet_length - padding_length - 1
//      byte[n2]  random padding; n2 = padding_length
//      byte[m]   mac (Message Authentication Code - MAC); m = mac_length
//==============================================================================
int make_ssh2_packet(buffer *orig_data, buffer *data_to_send,
		     SSH_CIPHER *cipher, SSH_MAC *mac, bool is_mac_,
		     uint32_t *seq)
{
	// 1. compute the padding length for padding.
	int non_padding_len = 4 + 1 + buffer_len(orig_data);

	int padding_block_len = 8;
	if (cipher->started && cipher->blkSize > padding_block_len)
		padding_block_len = cipher->blkSize;

	int padding_len =
		padding_block_len - (non_padding_len % padding_block_len);
	if (padding_len < 4)
		padding_len += padding_block_len;

	// 2. renew the output buffer.
	int total_len = non_padding_len + padding_len;
	if (is_mac_)
		total_len += mac->dgstSize;

	buffer_clear(data_to_send);

	// 3. Fill the output buffer.
	int packet_len = 1 + buffer_len(orig_data) + padding_len;

	buffer_append_be32(data_to_send, packet_len);
	buffer_append_byte(data_to_send, padding_len);
	buffer_append(data_to_send, buffer_data(orig_data),
		      buffer_len(orig_data));

	uint32_t padding[8]; /* padding at most 256 bits */
	for (int i = 0; i < 8; i++)
		padding[i] = rand();
	buffer_append(data_to_send, (const uint8_t *)padding, padding_len);

	// 4. Add MAC on the entire unencrypted packet,
	// including two length fields, 'payload' and 'random padding'.
	if (is_mac_) {
		const unsigned char *packet = buffer_data(data_to_send);
		int len = buffer_len(data_to_send);

		uint8_t digest[MAX_DGSTLEN];

		buffer mbuffer;
		buffer_init(&mbuffer);
		buffer_append_be32(&mbuffer, *seq);
		buffer_append(&mbuffer, packet, len);
		mac->getmac(mac, buffer_data(&mbuffer), buffer_len(&mbuffer),
			    digest);
		buffer_deinit(&mbuffer);

		buffer_append(data_to_send, digest, mac->dgstSize);
	}

	if (cipher->started) {
		// as RFC 4253:
		// When encryption is in effect, the packet length, padding
		// length, payload, and padding fields of each packet MUST be encrypted
		// with the given algorithm.

		uint8_t *data = buffer_data(data_to_send);
		int len = buffer_len(data_to_send) - mac->dgstSize;

		if (cipher->crypt(cipher, data, data, len) != 1)
			return -ECRYPT;
	}

	*seq = *seq + 1;
	return 0;
}
