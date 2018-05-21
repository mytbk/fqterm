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

#ifndef FQTERM_SSH2_KEX_H
#define FQTERM_SSH2_KEX_H

#include <openssl/sha.h>

#include "fqterm_ssh_kex.h"
#include "ssh_session.h"

namespace FQTerm {

class FQTermSSHPacketReceiver;
class FQTermSSHPacketSender;

class FQTermSSH2Kex: public FQTermSSHKex {
  Q_OBJECT;
private:
  enum FQTermSSH2KexState {
    BEFORE_KEXINIT, WAIT_REPLY, SESSIONKEY_SENT, KEYEX_OK
  }	kex_state_;

  /*
  string    V_C, the client's identification string (CR and LF
            excluded)
  string    V_S, the server's identification string (CR and LF
            excluded)
  string    I_C, the payload of the client's SSH_MSG_KEXINIT
  string    I_S, the payload of the server's SSH_MSG_KEXINIT
  string    K_S, the host key
  mpint     e, exchange value sent by the client
  mpint     f, exchange value sent by the server
  mpint     K, the shared secret
  */

  ssh_session sess;

  bool is_first_kex_;

  u_char cookie_[16];
  int server_flag_, ciphers_, auth_;
//  u_char session_id_[16];
  u_char session_key_[32];

  bool negotiateAlgorithms();
  void exchangeKey();
  bool verifyKey();
  void sendNewKeys();
  bool changeKeyAlg();

public:
  FQTermSSH2Kex(const char *V_C, const char *V_S);
  ~FQTermSSH2Kex();

  virtual void initKex(FQTermSSHPacketReceiver *packetReceiver,
                       FQTermSSHPacketSender *outputSender);
  void hostKeyHash(unsigned char *md)
  {
	  SHA256(sess.K_S, sess.K_S_len, md);
  }

  const unsigned char *K_S() { return sess.K_S; }
  int K_S_len() { return sess.K_S_len; }
public slots:
  void handlePacket(int type);
};

}  // namespace FQTerm

#endif //FQTERM_SSH2_KEX_H
