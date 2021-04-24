// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SSH1_PACKET_H
#define FQTERM_SSH1_PACKET_H

#include "fqterm_ssh_packet.h"
#include "ssh_error.h"

namespace FQTerm {

class FQTermSSH1PacketReceiver: public FQTermSSHPacketReceiver {
public:
  virtual void parseData(buffer *input);
  FQTermSSH1PacketReceiver();
};

}  // namespace FQTerm

#endif  // FQTERM_SSH1_PACKET
