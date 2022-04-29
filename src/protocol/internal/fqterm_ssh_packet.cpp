// SPDX-License-Identifier: GPL-2.0-or-later

#include "fqterm_trace.h"
#include "fqterm_ssh_packet.h"
#include "fqterm_ssh_const.h"

#include "crc32.h"
#include <openssl/bn.h>

namespace FQTerm {

//==============================================================================
//FQTermSSHPacketSender
//==============================================================================

FQTermSSHPacketSender::FQTermSSHPacketSender(int ver)
{
  buffer_init(&orig_data);
  buffer_init(&data_to_send);

  if (ver == 2)
	  cipher = &ssh_cipher_dummy;
  else
	  cipher = new_3des_ssh1(1);

  this->ver = ver;
  is_mac_ = false;
  mac = NULL;

  sequence_no_ = 0;
}

FQTermSSHPacketSender::~FQTermSSHPacketSender()
{
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
	uint8_t *buf = new uint8_t[bin_size];
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
	
	delete []buf;
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

void FQTermSSHPacketSender::startEncryption(const u_char *key, const u_char *IV)
{
	cipher->init(cipher, key, IV);
}

void FQTermSSHPacketSender::resetEncryption()
{
	cipher->started = false;
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

  cipher = &ssh_cipher_dummy;

  is_mac_ = false;
  mac = NULL;

  sequence_no_ = 0;
}

FQTermSSHPacketReceiver::~FQTermSSHPacketReceiver()
{
	buffer_deinit(&recvbuf);
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

uint8_t FQTermSSHPacketReceiver::getByte()
{
	if (buffer_len(&recvbuf) >= 1) {
		return buffer_get_u8(&recvbuf);
	} else {
		emit packetError("Read too many bytes!");
		return 0;
	}
}

uint32_t FQTermSSHPacketReceiver::getInt()
{
	if (buffer_len(&recvbuf) >= 4) {
		return buffer_get_u32(&recvbuf);
	} else {
		emit packetError("Read too many bytes!");
		return 0;
	}
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

void FQTermSSHPacketReceiver::startEncryption(const u_char *key, const u_char *IV)
{
	cipher->init(cipher, key, IV);
}

void FQTermSSHPacketReceiver::resetEncryption()
{
	cipher->started = false;
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
