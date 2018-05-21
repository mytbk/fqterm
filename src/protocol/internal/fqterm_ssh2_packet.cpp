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
#include "ssh_error.h"

namespace FQTerm {

void FQTermSSH2PacketReceiver::parseData(buffer *input)
{
	while (buffer_len(input) > 0) {
		int res = parse_ssh2_packet(input, &recvbuf, cipher,
				mac, is_mac_, &decrypted, &sequence_no_);
		switch (res) {
			case -ETOOSMALL:
				return;
			case -ETOOBIG:
				emit packetError("Packet too big!");
				return;
			case -ECRYPT:
				emit packetError("Decrypt error!");
				return;
			case -EMAC:
				emit packetError("MAC error!");
				return;
			default:
				real_data_len_ = res;
		}

		// 5. notify others a ssh packet is parsed successfully.
		packet_type_ = buffer_get_u8(&recvbuf);
		real_data_len_ -= 1;
		emit packetAvaliable(packet_type_);
	}
}

}  // namespace FQTerm
