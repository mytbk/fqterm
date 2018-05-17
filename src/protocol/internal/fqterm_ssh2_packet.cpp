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

#include "ssh_endian.h"
#include "buffer.h"
#include "ssh_packet.h"

namespace FQTerm {
//==============================================================================
//FQTermSSH2PacketSender
//==============================================================================
void FQTermSSH2PacketSender::makePacket()
{
	FQ_TRACE("ssh2packet", 9) << "----------------------------Send "
		<< (cipher->started ? "Encrypted": "plain")
		<< " Packet---->>>>>>>";

	// 0. compress
	if (is_compressed_)
		FQ_VERIFY(false);

	make_ssh2_packet(&orig_data, &data_to_send, cipher,
			mac, is_mac_, &sequence_no_);
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
    uint8_t padding_len = buffer_get_u8(input);
    real_data_len_ = packet_len - 1 - padding_len;
    buffer_clear(&recvbuf);
    buffer_append(&recvbuf, buffer_data(input), real_data_len_);
    buffer_consume(input, packet_len - 1);
    if (is_mac_)
      buffer_consume(input, mac->dgstSize);

    // 5. notify others a ssh packet is parsed successfully.
    packet_type_ = buffer_get_u8(&recvbuf);
    real_data_len_ -= 1;
    emit packetAvaliable(packet_type_);

    ++sequence_no_;
  }
}

}  // namespace FQTerm
