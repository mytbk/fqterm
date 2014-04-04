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

#include "fqterm_ssh_kex.h"
#include "fqterm_ssh_md5.h"
#include "fqterm_trace.h"

namespace FQTerm {

FQTermSSHKex::FQTermSSHKex(const char *V_C, const char *V_S) {
  V_C_ = new char[strlen(V_C) + 1];
  V_S_ = new char[strlen(V_S) + 1];

  strcpy(V_C_, V_C);
  strcpy(V_S_, V_S);
}

FQTermSSHKex::~FQTermSSHKex() {
  delete[] V_C_;
  delete[] V_S_;
}


//==============================================================================
//FQTermSSH1Kex
//==============================================================================

FQTermSSH1Kex::FQTermSSH1Kex(const char *V_C, const char *V_S)
    : FQTermSSHKex(V_C, V_S) {
  is_first_kex_ = true;
  kex_state_ = FQTermSSH1Kex::BEFORE_PUBLICKEY;
}

FQTermSSH1Kex::~FQTermSSH1Kex(){}

void FQTermSSH1Kex::initKex(FQTermSSHPacketReceiver *packetReceiver,
                            FQTermSSHPacketSender	*packetSender) {
  packet_receiver_ = packetReceiver;
  packet_sender_ = packetSender;
  packet_receiver_->disconnect(this);
  FQ_VERIFY(connect(packet_receiver_, SIGNAL(packetAvaliable(int)),
                    this, SLOT(handlePacket(int))));
  kex_state_ = FQTermSSH1Kex::BEFORE_PUBLICKEY;
  emit reKex();
}

void FQTermSSH1Kex::handlePacket(int type) {
  switch (kex_state_) {
    case FQTermSSH1Kex::BEFORE_PUBLICKEY:
      makeSessionKey();
      kex_state_ = FQTermSSH1Kex::SESSIONKEY_SENT;
      break;
    case FQTermSSH1Kex::SESSIONKEY_SENT:
      if (type != SSH1_SMSG_SUCCESS) {
        emit kexError(tr("Kex exchange failed!"));
        break;
      }
      emit kexOK();
      kex_state_ = FQTermSSH1Kex::KEYEX_OK;
      break;
    case FQTermSSH1Kex::KEYEX_OK:
      break;
  }
}

void FQTermSSH1Kex::makeSessionKey() {
  int i;
  BIGNUM *key;
  u_int32_t rand_val;
  int bits;
  int rbits;

  if (packet_receiver_->packetType() != SSH1_SMSG_PUBLIC_KEY) {
    emit kexError(tr("startKex: First packet is not public key"));
    return ;
  }
  packet_receiver_->getRawData((char*)cookie_, 8);

  // Get the public key.
  server_key_ = new FQTermSSHRSA;
  bits = packet_receiver_->getInt();
  packet_receiver_->getBN(server_key_->d_rsa->e);
  packet_receiver_->getBN(server_key_->d_rsa->n);

  rbits = BN_num_bits(server_key_->d_rsa->n);
  if (bits != rbits) {
    FQ_TRACE("sshkex", 0) << "Warning: Server lies about "
                          << "size of server public key: "
                          << "actual size: " << rbits
                          << " vs. anounced: " << bits;
    FQ_TRACE("sshkex", 0) << "Warning: This may be due to "
                          << "an old implementation of ssh.";
  }

  // Get the host key.
  host_key_ = new FQTermSSHRSA;
  bits = packet_receiver_->getInt();
  packet_receiver_->getBN(host_key_->d_rsa->e);
  packet_receiver_->getBN(host_key_->d_rsa->n);

  rbits = BN_num_bits(host_key_->d_rsa->n);
  if (bits != rbits) {
    FQ_TRACE("sshkex", 0) << "Warning: Server lies about "
                          << "size of server public key: "
                          << "actual size: " << rbits
                          << " vs. anounced: " << bits;
    FQ_TRACE("sshkex", 0) << "Warning: This may be due to "
                          << "an old implementation of ssh.";
  }

  // Get protocol flags.
  server_flag_ = packet_receiver_->getInt();
  ciphers_ = packet_receiver_->getInt();
  auth_ = packet_receiver_->getInt();

  if ((ciphers_ &(1 << SSH_CIPHER_3DES)) == 0) {
    FQ_VERIFY(false); // server do not support my cipher
  } 

  makeSessionId();

  // Generate an encryption key for the session. The key is a 256 bit
  // random number, interpreted as a 32-byte key, with the least
  // significant 8 bits being the first byte of the key.

  for (i = 0; i < 32; i++) {
    if (i % 4 == 0) {
      rand_val = rand();
    } 
    
    session_key_[i] = (rand_val &0xff);
    rand_val >>= 8;
  }

  key = BN_new();

  BN_set_word(key, 0);
  for (i = 0; i < 32; i++) {
    BN_lshift(key, key, 8);
    if (i < 16) {
      BN_add_word(key, session_key_[i] ^ session_id_[i]);
    } else {
      BN_add_word(key, session_key_[i]);
    } 
  }

  if (BN_cmp(server_key_->d_rsa->n, host_key_->d_rsa->n) < 0) {
    server_key_->publicEncrypt(key, key);
    host_key_->publicEncrypt(key, key);
  } else {
    host_key_->publicEncrypt(key, key);
    server_key_->publicEncrypt(key, key);
  }

  delete host_key_;
  delete server_key_;

  packet_sender_->startPacket(SSH1_CMSG_SESSION_KEY);
  packet_sender_->putByte(SSH_CIPHER_3DES);
  packet_sender_->putRawData((const char*)cookie_, 8);
  packet_sender_->putBN(key);

  BN_free(key);

  packet_sender_->putInt(1);
  packet_sender_->write();

  emit startEncryption(session_key_);
}

void FQTermSSH1Kex::makeSessionId() {
  u_char *p;
  FQTermSSHMD5 *md5;
  int servlen, hostlen;

  md5 = new FQTermSSHMD5;
  servlen = BN_num_bytes(server_key_->d_rsa->n);
  hostlen = BN_num_bytes(host_key_->d_rsa->n);

  p = new u_char[servlen + hostlen];

  BN_bn2bin(host_key_->d_rsa->n, p);
  BN_bn2bin(server_key_->d_rsa->n, p + hostlen);
  md5->update(p, servlen + hostlen);
  md5->update(cookie_, 8);
  md5->final(session_id_);
  delete md5;
  delete [] p;
}

}  // namespace FQTerm

#include "fqterm_ssh_kex.moc"
