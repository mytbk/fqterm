// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SSH_PACKET_H
#define FQTERM_SSH_PACKET_H

#include <openssl/bn.h>

#include <QObject>

#include "fqterm_ssh_types.h"
#include "ssh_mac.h"
#include "ssh_cipher.h"
#include "buffer.h"
#include "ssh_packet.h"

namespace FQTerm {

class FQTermSSHPacketSender: public QObject {
  Q_OBJECT;
 public:
  buffer orig_data; /* always unencrypted */
  buffer data_to_send;
  ssh_cipher_t *cipher;
  ssh_mac_t *mac;
  bool is_mac_;
  uint32_t sequence_no_;

  int ver;

  FQTermSSHPacketSender(int);
  virtual ~FQTermSSHPacketSender();

  void startPacket(uint8_t pkt_type);
  inline void putByte(uint8_t b) { buffer_append_byte(&orig_data, b); }
  inline void putInt(uint32_t x) { buffer_append_be32(&orig_data, x); }
  void putString(const char *string, int len = -1);
  inline void putRawData(const uint8_t *data, size_t len)
  { buffer_append(&orig_data, data, len); }
  void putBN(BIGNUM *bignum);
  void write();

  virtual int getIVSize() const { return cipher->IVSize;}
  virtual int getKeySize() const { return cipher->keySize;}
  int getMacKeySize() const { return mac->keySize;}

  void startEncryption(const u_char *key, const u_char *IV = NULL);
  void startMac(const u_char *sessionkey);
  void resetMac();
  inline void makePacket()
  {
	  if (ver == 2) {
		  make_ssh2_packet(&orig_data, &data_to_send, cipher,
				  mac, is_mac_, &sequence_no_);
	  } else {
		  make_ssh1_packet(&orig_data, &data_to_send, cipher);
	  }
  }
 public slots:
  void resetEncryption();

 signals:
  void dataToWrite();
};

class FQTermSSHPacketReceiver: public QObject {
  Q_OBJECT;
 public:
  buffer recvbuf;
  ssh_cipher_t *cipher;
  ssh_mac_t *mac;
  bool is_mac_;
  uint32_t sequence_no_;

  FQTermSSHPacketReceiver();
  virtual ~FQTermSSHPacketReceiver();

  int packetType()const {
    return packet_type_;
  }

  u_char getByte();
  u_int getInt();
  void *getString(int *length = NULL);
  void getRawData(char *data, int length);
  void getBN(BIGNUM *bignum);
  void consume(int len);

  virtual int packetDataLen() const { return real_data_len_;}
  virtual int getIVSize() const { return cipher->IVSize;}
  virtual int getKeySize() const { return cipher->keySize;}
  int getMacKeySize() const { return mac->keySize;}

  virtual void parseData(buffer *input) = 0;
  void startEncryption(const u_char *key, const u_char *IV = NULL);
  void startMac(const u_char *sessionkey);
  void resetMac();
 public slots:
  void resetEncryption();

 signals:
  void packetAvaliable(int type);
  void packetError(QString);

 protected:
  int packet_type_;
  int real_data_len_;
};

}  // namespace FQTerm

#endif  // FQTERM_SSH_PACKET
