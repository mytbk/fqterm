// SPDX-License-Identifier: GPL-2.0-or-later

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
