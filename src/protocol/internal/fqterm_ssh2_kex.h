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

  int I_C_len_;
  char *I_C_;
  int I_S_len_;
  char *I_S_;

  BIGNUM *bn_x_;
  BIGNUM *bn_e_;
  BIGNUM *bn_g_;
  BIGNUM *bn_p_;
  BN_CTX *ctx_;

  BIGNUM *bn_K_;
  BIGNUM *bn_f_;

  unsigned char H_[SHA_DIGEST_LENGTH];

  unsigned char *session_id_;




  bool is_first_kex_;

  ssh_pubkey_t *host_key_;
  ssh_pubkey_t *server_key_;

  u_char cookie_[16];
  int server_flag_, ciphers_, auth_;
//  u_char session_id_[16];
  u_char session_key_[32];
  
  void negotiateAlgorithms();
  void exchangeKey();
  bool verifyKey();
  void sendNewKeys();
  bool changeKeyAlg();

  unsigned char *computeKey(int len, char flag);

public:
  FQTermSSH2Kex(const char *V_C, const char *V_S);
  ~FQTermSSH2Kex();

  virtual void initKex(FQTermSSHPacketReceiver *packetReceiver,
                       FQTermSSHPacketSender *outputSender);

public slots:
  void handlePacket(int type);
};

}  // namespace FQTerm

#endif //FQTERM_SSH2_KEX_H
