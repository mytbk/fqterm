// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SSH_KEX_H
#define FQTERM_SSH_KEX_H

#include <openssl/bn.h>
#include <QObject>

#include "fqterm_ssh_types.h"
#include "fqterm_ssh_packet.h"
#include "fqterm_ssh_const.h"
#include "ssh_pubkey_crypto.h"

namespace FQTerm {

class FQTermSSHPacketReceiver;
class FQTermSSHPacketSender;

class FQTermSSHKex: public QObject {
  Q_OBJECT;
protected:
  FQTermSSHPacketReceiver *packet_receiver_;
  FQTermSSHPacketSender *packet_sender_;

  char *V_C_;
  char *V_S_;
public:
  // Init with 
  //    string    V_C, the client's identification string (CR and LF
  //              excluded)
  //    string    V_S, the server's identification string (CR and LF
  //              excluded)
  FQTermSSHKex(const char *V_C, const char *V_S);
  virtual ~FQTermSSHKex();

  virtual void initKex(FQTermSSHPacketReceiver *packetReceiver,
                       FQTermSSHPacketSender *outputSender) = 0;
  virtual void hostKeyHash(unsigned char *md) {}
public slots:
  virtual void handlePacket(int type) = 0;

signals:
  void kexOK();
  void reKex();
  void kexError(QString);
};

class FQTermSSH1Kex: public FQTermSSHKex {
  Q_OBJECT;
private:
  enum FQTermSSH1KexState {
    BEFORE_PUBLICKEY, SESSIONKEY_SENT, KEYEX_OK
  }	kex_state_;

  bool is_first_kex_;

  struct ssh_pubkey_t *host_key_;
  struct ssh_pubkey_t *server_key_;

  u_char cookie_[8];
  int server_flag_, ciphers_, auth_;
  u_char session_id_[16];
  u_char session_key_[32];
  
  void makeSessionId();
  void makeSessionKey();

public:
  FQTermSSH1Kex(const char *V_C, const char *V_S);
  ~FQTermSSH1Kex();
  virtual void initKex(FQTermSSHPacketReceiver *packetReceiver,
                       FQTermSSHPacketSender *outputSender);

  virtual void handlePacket(int type);
};

}  // namespace FQTerm

#endif //FQTERM_SSH_KEX_H
