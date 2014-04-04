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
#include "fqterm_ssh_buffer.h"
#include "fqterm_ssh_mac.h"
#include "fqterm_ssh_cipher.h"
#include "fqterm_serialization.h"

namespace FQTerm {

class FQTermSSHPacketSender: public QObject {
  Q_OBJECT;
 public:
  FQTermSSHBuffer *output_buffer_;
  FQTermSSHBuffer *buffer_;

  FQTermSSHPacketSender();
  virtual ~FQTermSSHPacketSender();
  
  void startPacket(int pkt_type);
  void putByte(int data);
  void putInt(u_int data);
  void putString(const char *string, int len = -1);
  void putRawData(const char *data, int length);
  void putBN(BIGNUM *bignum);
  void putBN2(BIGNUM *bignum);
  void write();

  int getIVSize() const { return cipher_->getIVSize();}
  int getKeySize() const { return cipher_->getKeySize();}
  int getMacKeySize() const { return mac_->keySize();}

 public slots:
  virtual void setEncryptionType(int cipherType);
  void startEncryption(const u_char *key, const u_char *IV = NULL);
  void resetEncryption();

  void setMacType(int macType);
  void startMac(const u_char *sessionkey);
  void resetMac();

  void enableCompress(int enable) {is_compressed_ = enable;};

 signals:
  void dataToWrite();

 protected:
  bool is_encrypt_;
  int cipher_type_;
  FQTermSSHCipher *cipher_;

  bool is_mac_;
  int mac_type_;
  FQTermSSHMac *mac_;

  bool is_compressed_;

  u_int32_t sequence_no_;

  virtual void makePacket() = 0;
};

class FQTermSSHPacketReceiver: public QObject {
  Q_OBJECT;
 public:
  FQTermSSHBuffer *buffer_;

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
  void getBN2(BIGNUM *bignum);
  void consume(int len);

  virtual int packetDataLen() const { return real_data_len_;}
  int getIVSize() const { return cipher_->getIVSize();}
  int getKeySize() const { return cipher_->getKeySize();}
  int getMacKeySize() const { return mac_->keySize();}

  virtual void parseData(FQTermSSHBuffer *input) = 0;
 public slots:
  virtual void setEncryptionType(int cipherType);
  void startEncryption(const u_char *key, const u_char *IV = NULL);
  void resetEncryption();

  void setMacType(int macType);
  void startMac(const u_char *sessionkey);
  void resetMac();

  void enableCompress(int enable) {is_compressed_ = enable;};

 signals:
  void packetAvaliable(int type);
  void packetError(QString);

 protected:
  bool is_decrypt_;
  int cipher_type_;
  FQTermSSHCipher *cipher_;

  bool is_mac_;
  int mac_type_;
  FQTermSSHMac *mac_;

  bool is_compressed_;

  int packet_type_;
  int real_data_len_;

  u_int32_t sequence_no_;
};

}  // namespace FQTerm

#endif  // FQTERM_SSH_PACKET
