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

#include "fqterm_trace.h"
#include "fqterm_ssh_buffer.h"
#include "fqterm_ssh_packet.h"
#include "fqterm_ssh_des.h"

#include "fqterm_serialization.h"
#include "crc32.h"

namespace FQTerm {

//==============================================================================
//FQTermSSHPacketSender
//==============================================================================

FQTermSSHPacketSender::FQTermSSHPacketSender() {
  buffer_ = new FQTermSSHBuffer(1024);
  output_buffer_ = new FQTermSSHBuffer(1024);

  is_encrypt_ = false;
  cipher_type_ = SSH_CIPHER_NONE;
  cipher_ = NULL;
  setEncryptionType(SSH_CIPHER_3DES);

  is_mac_ = false;
  mac_type_ = FQTERM_SSH_MAC_NONE;
  mac_ = NULL;

  is_compressed_ = false;

  sequence_no_ = 0;
}

FQTermSSHPacketSender::~FQTermSSHPacketSender() {
  delete buffer_;
  delete output_buffer_;
  if (is_encrypt_) {
    delete cipher_;
  } 
}

void FQTermSSHPacketSender::putRawData(const char *data, int len) {
  buffer_->putRawData(data, len);
}

void FQTermSSHPacketSender::putByte(int data) {
  buffer_->putByte(data);
}

void FQTermSSHPacketSender::putInt(u_int data) {
  buffer_->putInt(data);
}

void FQTermSSHPacketSender::putString(const char *string, int len) {
  buffer_->putString(string, len);
}

void FQTermSSHPacketSender::putBN(BIGNUM *bn) {
  buffer_->putSSH1BN(bn);
}

void FQTermSSHPacketSender::putBN2(BIGNUM *bn) {
  buffer_->putSSH2BN(bn);
}

void FQTermSSHPacketSender::startPacket(int pkt_type) {
  buffer_->clear();
  buffer_->putByte(pkt_type);
}

void FQTermSSHPacketSender::write() {
  makePacket();
  emit dataToWrite();
}

void FQTermSSHPacketSender::setEncryptionType(int cipherType) {
  cipher_type_ = cipherType;

  delete cipher_;
  cipher_ = NULL;

  switch (cipher_type_) {
    case SSH_CIPHER_3DES:
      cipher_ = new FQTermSSH1DES3;
      break;
  }
}

void FQTermSSHPacketSender::startEncryption(const u_char *key, const u_char *IV) {
  is_encrypt_ = true;
  cipher_->setIV(IV);
  cipher_->setKey(key);
}

void FQTermSSHPacketSender::resetEncryption() {
  is_encrypt_ = false;
}

void FQTermSSHPacketSender::setMacType(int macType) {
  mac_type_ = macType;

  delete mac_;
  mac_ = NULL;

  switch (mac_type_) {
    case FQTERM_SSH_HMAC_SHA1:
      mac_ = new FQTermSSHSHA1;
      break;
  }
}

void FQTermSSHPacketSender::startMac(const u_char *key) {
  is_mac_ = true;
  mac_->setKey(key);
}

void FQTermSSHPacketSender::resetMac() {
  is_mac_ = false;
}

//==============================================================================
//FQTermSSHPacketReceiver
//==============================================================================

FQTermSSHPacketReceiver::FQTermSSHPacketReceiver() {
  buffer_ = new FQTermSSHBuffer(1024);

  is_decrypt_ = false;
  cipher_type_ = SSH_CIPHER_NONE;
  cipher_ = NULL;
  setEncryptionType(SSH_CIPHER_3DES);

  is_mac_ = false;
  mac_type_ = FQTERM_SSH_MAC_NONE;
  mac_ = NULL;

  is_compressed_ = false;

  sequence_no_ = 0;
}

FQTermSSHPacketReceiver::~FQTermSSHPacketReceiver() {
  delete buffer_;
  if (is_decrypt_) {
    delete cipher_;
  } 
}

void FQTermSSHPacketReceiver::getRawData(char *data, int length) {
  buffer_->getRawData(data, length);
}

u_char FQTermSSHPacketReceiver::getByte() {
  return buffer_->getByte();
}

u_int FQTermSSHPacketReceiver::getInt() {
  return buffer_->getInt();
}

void *FQTermSSHPacketReceiver::getString(int *length) {
  return buffer_->getString(length);
}

void FQTermSSHPacketReceiver::getBN(BIGNUM *bignum) {
  buffer_->getSSH1BN(bignum);
}

void FQTermSSHPacketReceiver::getBN2(BIGNUM *bignum) {
  buffer_->getSSH2BN(bignum);
}

void FQTermSSHPacketReceiver::consume(int len) {
  buffer_->consume(len);
}

void FQTermSSHPacketReceiver::setEncryptionType(int cipherType) {
  cipher_type_ = cipherType;

  delete cipher_;
  cipher_ = NULL;

  switch (cipher_type_) {
    case SSH_CIPHER_3DES:
      cipher_ = new FQTermSSH1DES3;
      break;
  }
}

void FQTermSSHPacketReceiver::startEncryption(const u_char *key, const u_char *IV) {
  is_decrypt_ = true;
  cipher_->setIV(IV);
  cipher_->setKey(key);
}

void FQTermSSHPacketReceiver::resetEncryption() {
  is_decrypt_ = false;
}

void FQTermSSHPacketReceiver::setMacType(int macType) {
  mac_type_ = macType;
  delete mac_;
  mac_ = NULL;

  switch (mac_type_) {
    case FQTERM_SSH_HMAC_SHA1:
      mac_ = new FQTermSSHSHA1;
      break;
  }
}

void FQTermSSHPacketReceiver::startMac(const u_char *key) {
  is_mac_ = true;
  mac_->setKey(key);
}

void FQTermSSHPacketReceiver::resetMac() {
  is_mac_ = false;
}

}  // namespace FQTerm

#include "fqterm_ssh_packet.moc"
