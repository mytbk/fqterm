// SPDX-License-Identifier: GPL-2.0-or-later

#include "fqterm_trace.h"
#include "fqterm_ssh1_packet.h"

#include "ssh_endian.h"
#include "crc32.h"
#include "ssh_packet.h"

namespace FQTerm {
//==============================================================================
//FQTermSSH1PacketReceiver
//==============================================================================

	FQTermSSH1PacketReceiver::FQTermSSH1PacketReceiver()
	{
		cipher = new_3des_ssh1(0);
	}

void FQTermSSH1PacketReceiver::parseData(buffer *input)
{
	while (buffer_len(input) > 0) {
		int res = parse_ssh1_packet(input, &recvbuf, cipher);
		switch (res) {
			case -ETOOBIG:
				emit packetError("Packet too big!");
				return;
			case -ETOOSMALL:
				return;
			case -ECRC32:
				emit packetError("CRC32 error!");
				return;
			default:
				real_data_len_ = res;
		}

		packet_type_ = buffer_get_u8(&recvbuf);

		emit packetAvaliable(packet_type_);
	}
}

}  // namespace FQTerm
