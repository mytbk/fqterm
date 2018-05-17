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

#ifndef FQTERM_SSH_PACKET_H
#define FQTERM_SSH_PACKET_H

#include <openssl/bn.h>

#include <QObject>

#include "fqterm_ssh_types.h"
#include "ssh_mac.h"
#include "fqterm_serialization.h"
#include "ssh_cipher.h"
#include "buffer.h"

namespace FQTerm {

class FQTermSSHPacketSender: public QObject {
  Q_OBJECT;
 public:
  buffer orig_data; /* always unencrypted */
  buffer data_to_send;
  ssh_cipher_t *cipher;
  ssh_mac_t *mac;

  FQTermSSHPacketSender();
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
 public slots:
  void resetEncryption();

 signals:
  void dataToWrite();

 protected:
  bool is_mac_;

  bool is_compressed_;

  u_int32_t sequence_no_;

  virtual void makePacket() = 0;
};

class FQTermSSHPacketReceiver: public QObject {
  Q_OBJECT;
 public:
  buffer recvbuf;
  ssh_cipher_t *cipher;
  ssh_mac_t *mac;

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
  bool is_mac_;

  bool is_compressed_;

  int packet_type_;
  int real_data_len_;

  u_int32_t sequence_no_;
};

}  // namespace FQTerm

#endif  // FQTERM_SSH_PACKET
