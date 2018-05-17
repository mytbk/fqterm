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
#include "fqterm_ssh1_packet.h"

#include "fqterm_serialization.h"
#include "crc32.h"
#include "ssh_packet.h"

namespace FQTerm {
//==============================================================================
//FQTermSSH1PacketSender
//==============================================================================

	FQTermSSH1PacketSender::FQTermSSH1PacketSender()
	{
		cipher = new_3des_ssh1(1);
	}

void FQTermSSH1PacketSender::makePacket()
{
	make_ssh1_packet(&orig_data, &data_to_send, cipher);
}

//==============================================================================
//FQTermSSH1PacketReceiver
//==============================================================================

	FQTermSSH1PacketReceiver::FQTermSSH1PacketReceiver()
	{
		cipher = new_3des_ssh1(0);
	}

void FQTermSSH1PacketReceiver::parseData(buffer *input) {
  u_int mycrc, gotcrc;
  u_char *buf = NULL;
  u_char *targetData = NULL;
  u_char *sourceData = NULL;

  // Get the length of the packet.
  while (buffer_len(input) > 0) {
    if (buffer_len(input) < 4) {
      FQ_TRACE("ssh1packet", 3) << "The packet is too small.";
      return ;
    }
    buf = buffer_data(input);
    real_data_len_ = ntohu32(buf);

    if (real_data_len_ > SSH_BUFFER_MAX) {
      emit packetError(tr("parseData: The packet is too big"));
      return ;
    }

    u_int total_len = (real_data_len_ + 8) &~7;
    u_int padding_len = total_len - real_data_len_;

    real_data_len_ -= 5;
    buffer_clear(&recvbuf);

    // Get the data of the packet.
    if (buffer_len(input) - 4 < (long)total_len) {
      FQ_TRACE("ssh1packet", 3) << "The packet is too small";
      return ;
    }

    real_data_len_ = buffer_get_u32(input) - 5;
    targetData = new u_char[total_len];
    sourceData = new u_char[total_len];
    memset(targetData, 0, total_len);
    memset(sourceData, 0, total_len);

    buffer_get(input, sourceData, total_len);
    if (cipher->started) {
	    cipher->crypt(cipher, sourceData, targetData, total_len);
    } else {
	    memcpy(targetData, sourceData, total_len);
    }

    buffer_append(&recvbuf, targetData, total_len);

    // Check the crc32.
    buf = buffer_data(&recvbuf) + total_len - 4;
    mycrc = ntohu32(buf);
    gotcrc = ssh_crc32(buffer_data(&recvbuf), total_len - 4);

    if (mycrc != gotcrc) {
      emit packetError(tr("parseData: bad CRC32"));
      break;
    }

    // Drop the padding.
    buffer_consume(&recvbuf, padding_len);

    packet_type_ = buffer_get_u8(&recvbuf);

    emit packetAvaliable(packet_type_);

    delete [] sourceData;
    delete [] targetData;
  }
}

}  // namespace FQTerm
