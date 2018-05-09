/***************************************************************************
 *   fqterm, a terminal emulator for both BBS and *nix.                    *
 *   Copyright (C) 2008 fqterm development group.                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.               *
 ***************************************************************************/

#include "fqterm_trace.h"
#include "fqterm_ssh_buffer.h"
#include "fqterm_ssh_packet.h"
#include "fqterm_ssh_const.h"

#include "fqterm_serialization.h"
#include "crc32.h"
#include <openssl/bn.h>

namespace FQTerm {

//==============================================================================
//FQTermSSHPacketSender
//==============================================================================

FQTermSSHPacketSender::FQTermSSHPacketSender()
{
  buffer_init(&orig_data);
  buffer_init(&data_to_send);

  is_encrypt_ = false;
  cipher_type_ = SSH_CIPHER_NONE;
  cipher = NULL;

  is_mac_ = false;
  mac = NULL;

  is_compressed_ = false;

  sequence_no_ = 0;
}

FQTermSSHPacketSender::~FQTermSSHPacketSender()
{
	if (cipher)
		cipher->cleanup(cipher);
        if (mac)
		mac->cleanup(mac);
	buffer_deinit(&data_to_send);
	buffer_deinit(&orig_data);
}

void FQTermSSHPacketSender::putString(const char *s, int len)
{
	if (len < 0)
		len = strlen(s);

	putInt(len);
	putRawData((const uint8_t *)s, len);
}

void FQTermSSHPacketSender::putBN(BIGNUM *bignum)
{
	int bits = BN_num_bits(bignum);
	int bin_size = (bits + 7) / 8;
	uint8_t buf[bin_size];
	int oi;
	uint8_t msg[2];

	// Get the value of in binary
	oi = BN_bn2bin(bignum, buf);
	if (oi != bin_size) {
		FQ_TRACE("sshbuffer", 0) << "BN_bn2bin() failed: oi = " << oi
			<< " != bin_size." << bin_size;
	} 

	// Store the number of bits in the buffer in two bytes, msb first
	buffer_append_be16(&orig_data, bits);
	// Store the binary data.
	putRawData(buf, oi);
}

void FQTermSSHPacketSender::startPacket(uint8_t pkt_type)
{
	buffer_clear(&orig_data);
	putByte(pkt_type);
}

void FQTermSSHPacketSender::write()
{
	makePacket();
	emit dataToWrite();
}

void FQTermSSHPacketSender::startEncryption(const u_char *key, const u_char *IV) {
	is_encrypt_ = true;

	if (cipher!=NULL) {
		memcpy(cipher->IV, IV, cipher->IVSize);
		memcpy(cipher->key, key, cipher->keySize);
		cipher->init(cipher);
	}
}

void FQTermSSHPacketSender::resetEncryption() {
  is_encrypt_ = false;
}

void FQTermSSHPacketSender::startMac(const u_char *key) {
  is_mac_ = true;
  memcpy(mac->key, key, mac->keySize);
}

void FQTermSSHPacketSender::resetMac() {
  is_mac_ = false;
}

//==============================================================================
//FQTermSSHPacketReceiver
//==============================================================================

FQTermSSHPacketReceiver::FQTermSSHPacketReceiver()
{
	buffer_init(&recvbuf);

  is_decrypt_ = false;
  cipher_type_ = SSH_CIPHER_NONE;
  cipher = NULL;

  is_mac_ = false;
  mac = NULL;

  is_compressed_ = false;

  sequence_no_ = 0;
}

FQTermSSHPacketReceiver::~FQTermSSHPacketReceiver()
{
	buffer_deinit(&recvbuf);
	if (cipher)
		cipher->cleanup(cipher);
        if (mac)
		mac->cleanup(mac);
}

void FQTermSSHPacketReceiver::getRawData(char *data, int length)
{
	if (buffer_len(&recvbuf) >= length)
		buffer_get(&recvbuf, (uint8_t*)data, length);
	else
		emit packetError("Read too many bytes!");
}

u_char FQTermSSHPacketReceiver::getByte()
{
	if (buffer_len(&recvbuf) >= 1)
		return buffer_get_u8(&recvbuf);
	else
		emit packetError("Read too many bytes!");
}

u_int FQTermSSHPacketReceiver::getInt()
{
	if (buffer_len(&recvbuf) >= 4)
		return buffer_get_u32(&recvbuf);
	else
		emit packetError("Read too many bytes!");
}

void *FQTermSSHPacketReceiver::getString(int *length)
{
	uint32_t l = getInt();
	char *data = new char[l+1];
	getRawData(data, l);
	data[l] = 0;
	if (length != NULL)
		*length = l;
	return data;
}

void FQTermSSHPacketReceiver::getBN(BIGNUM *bignum)
{
	int bits, bytes;
	u_char buf[2];
	u_char *bin;

	// Get the number for bits.
	if (buffer_len(&recvbuf) >= 2) {
		bits = buffer_get_u16(&recvbuf);
	} else {
		emit packetError("Read too many bytes!");
		return;
	}
	// Compute the number of binary bytes that follow.
	bytes = (bits + 7) / 8;
	if (bytes > 8 *1024) {
		emit packetError("Can't handle BN of size!");
		return ;
	}
	if (buffer_len(&recvbuf) < bytes) {
		emit packetError("The input buffer is too small!");
		return ;
	}
	bin = buffer_data(&recvbuf);
	BN_bin2bn(bin, bytes, bignum);
	buffer_consume(&recvbuf, bytes);
}

void FQTermSSHPacketReceiver::consume(int len)
{
	buffer_consume(&recvbuf, len);
}

void FQTermSSHPacketReceiver::startEncryption(const u_char *key, const u_char *IV) {
	is_decrypt_ = true;

	if (cipher!=NULL) {
		memcpy(cipher->IV, IV, cipher->IVSize);
		memcpy(cipher->key, key, cipher->keySize);
		cipher->init(cipher);
	}
}

void FQTermSSHPacketReceiver::resetEncryption() {
  is_decrypt_ = false;
}

void FQTermSSHPacketReceiver::startMac(const u_char *key) {
  is_mac_ = true;
  memcpy(mac->key, key, mac->keySize);
}

void FQTermSSHPacketReceiver::resetMac() {
  is_mac_ = false;
}

}  // namespace FQTerm

#include "fqterm_ssh_packet.moc"
