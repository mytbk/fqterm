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
#include "fqterm_ssh2_packet.h"

#include "fqterm_serialization.h"
#include "buffer.h"

namespace FQTerm {
//==============================================================================
//FQTermSSH2PacketSender
//==============================================================================
// SSH2 Packet Structure:
//      uint32    packet_length
//      byte      padding_length
//      byte[n1]  payload; n1 = packet_length - padding_length - 1
//      byte[n2]  random padding; n2 = padding_length
//      byte[m]   mac (Message Authentication Code - MAC); m = mac_length
//==============================================================================

void FQTermSSH2PacketSender::makePacket()
{
	FQ_TRACE("ssh2packet", 9) << "----------------------------Send "
		<< (cipher->started ? "Encrypted": "plain")
		<< " Packet---->>>>>>>";

	// 0. compress
	if (is_compressed_)
		FQ_VERIFY(false);

	// 1. compute the padding length for padding.
	int non_padding_len = 4 + 1 + buffer_len(&orig_data);

	int padding_block_len = 8;
	if (cipher->started && cipher->blkSize > padding_block_len)
		padding_block_len = cipher->blkSize;

	int padding_len = padding_block_len - (non_padding_len % padding_block_len);
	if (padding_len < 4)
		padding_len += padding_block_len;

	// 2. renew the output buffer.
	int total_len = non_padding_len + padding_len;
	if (is_mac_)
		total_len += mac->dgstSize;

	buffer_clear(&data_to_send);

	// 3. Fill the output buffer.
	int packet_len = 1 + buffer_len(&orig_data) + padding_len;

	buffer_append_be32(&data_to_send, packet_len);
	buffer_append_byte(&data_to_send, padding_len);
	buffer_append(&data_to_send, buffer_data(&orig_data), buffer_len(&orig_data));

	uint32_t padding[8]; /* padding at most 256 bits */
	for (int i = 0; i < 8; i++)
		padding[i] = rand();
	buffer_append(&data_to_send, (const uint8_t*)padding, padding_len);

	// 4. Add MAC on the entire unencrypted packet,
	// including two length fields, 'payload' and 'random padding'.
	if (is_mac_) {
		const unsigned char *packet = buffer_data(&data_to_send);
		int len = buffer_len(&data_to_send);

		uint8_t digest[MAX_DGSTLEN];

		buffer mbuffer;
		buffer_init(&mbuffer);
		buffer_append_be32(&mbuffer, sequence_no_);
		buffer_append(&mbuffer, packet, len);
		mac->getmac(mac, buffer_data(&mbuffer), buffer_len(&mbuffer), digest);
		buffer_deinit(&mbuffer);

		buffer_append(&data_to_send, digest, mac->dgstSize);
	}

	if (is_compressed_) {
		FQ_VERIFY(false);
	}

	if (cipher->started) {
		// as RFC 4253:
		// When encryption is in effect, the packet length, padding
		// length, payload, and padding fields of each packet MUST be encrypted
		// with the given algorithm.

		uint8_t *data = buffer_data(&data_to_send);
		int len = buffer_len(&data_to_send) - mac->dgstSize;

		FQ_TRACE("ssh2packet", 9) << "An packet (without MAC) to be encrypted:" 
			<< len << " bytes:\n" 
			<< dumpHexString << std::string((const char *)data, len);

		FQ_VERIFY(cipher->crypt(cipher, data, data, len)==1);

		FQ_TRACE("ssh2packet", 9) << "An encrypted packet (without MAC) made:" 
			<< len << " bytes:\n" 
			<< dumpHexString << std::string((const char *)data, len);
	} 

	++sequence_no_;
}

//==============================================================================
//FQTermSSH2PacketReceiver
//==============================================================================
void FQTermSSH2PacketReceiver::parseData(buffer *input) {
  FQ_TRACE("ssh2packet", 9) << "----------------------------Receive "
                            << (cipher->started ? "Encrypted": "plain")
                            << " Packet----<<<<<<<";
  while (buffer_len(input) > 0) {
    // 1. Check the ssh packet
    if (buffer_len(input) < 16
        || (cipher->started && buffer_len(input) < cipher->blkSize)
        || buffer_len(input) < last_expected_input_length_
        ) {
      FQ_TRACE("ssh2packet", 3)
          << "Got an incomplete packet. Wait for more data.";
      return ;
    }

    if (last_expected_input_length_ == 0) {
      if (cipher->started) {
			// decrypte the first block to get the packet_length field.
			FQ_VERIFY(cipher->crypt(cipher, buffer_data(input), buffer_data(input), cipher->blkSize)==1);
      }
    } else {
      // last_expected_input_length_ != 0
      // indicates an incomplete ssh2 packet received last time,
      // the first block of data is already decrypted at that time,
      // so it must not be decrypted again.
    }

    int packet_len = ntohu32(buffer_data(input));

    if (packet_len > SSH_BUFFER_MAX) {
      emit packetError(tr("parseData: packet too big"));
      return ;
    }

    int expected_input_len = 4 + packet_len + (is_mac_ ? mac->dgstSize : 0);

    if (buffer_len(input)  < (long)expected_input_len) {
      FQ_TRACE("ssh2packet", 3)
          << "The packet is too small. Wait for more data.";
      last_expected_input_length_ = expected_input_len;    
      return ;
    } else {
      last_expected_input_length_ = 0;      
    }

    // 2. decrypte data.
    if (cipher->started) {
      // decrypte blocks left.
      unsigned char *tmp = buffer_data(input) + cipher->blkSize;
      int left_len = expected_input_len - cipher->blkSize - mac->dgstSize;
      FQ_VERIFY(cipher->crypt(cipher, tmp, tmp, left_len)==1);
    }

	// 3. check MAC
    if (is_mac_) {
	    int digest_len = mac->dgstSize;
	    uint8_t digest[MAX_DGSTLEN];

	    buffer mbuf;
	    buffer_init(&mbuf);
	    buffer_append_be32(&mbuf, sequence_no_);
	    buffer_append(&mbuf, (const uint8_t*)buffer_data(input),
			    expected_input_len - digest_len);
	    mac->getmac(mac, buffer_data(&mbuf), buffer_len(&mbuf), digest);
	    buffer_deinit(&mbuf);

	    u_char *received_digest = buffer_data(input) + expected_input_len - digest_len;

	    if (memcmp(digest, received_digest, digest_len) != 0) {
		    emit packetError("incorrect MAC.");
		    return ;
	    }
    }

    // 4. get every field of the ssh packet.
    packet_len = buffer_get_u32(input);

    std::vector<u_char> data(packet_len);

    buffer_get(input, &data[0], packet_len);
    if (is_mac_)
      buffer_consume(input, mac->dgstSize);

    int padding_len = data[0];

    real_data_len_ = packet_len - 1 - padding_len;

    buffer_clear(&recvbuf);
    buffer_append(&recvbuf, &data[0] + 1, real_data_len_);

    FQ_TRACE("ssh2packet", 9) << "Receive " << real_data_len_ << " bytes payload:\n" 
                              << dumpHexString << std::string((char *)&data[0] + 1, real_data_len_);

    // 5. notify others a ssh packet is parsed successfully.
    packet_type_ = buffer_get_u8(&recvbuf);
    real_data_len_ -= 1;
    emit packetAvaliable(packet_type_);

    ++sequence_no_;
  }
}

}  // namespace FQTerm
