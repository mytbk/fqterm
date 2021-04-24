// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SSH2_PACKET_H
#define FQTERM_SSH2_PACKET_H

#include "fqterm_ssh_packet.h"
#include "buffer.h"

namespace FQTerm {

class FQTermSSH2PacketReceiver: public FQTermSSHPacketReceiver
{
private:
	uint32_t decrypted;
public:
FQTermSSH2PacketReceiver() : decrypted(0) { }

	virtual void parseData(buffer *input);
};

}  // namespace FQTerm

#endif  // FQTERM_SSH2_PACKET
