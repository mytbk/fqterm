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

#ifndef FQTERM_SSH_DES_H
#define FQTERM_SSH_DES_H

#include <openssl/ssl.h>
#include <openssl/des.h>

#include "fqterm_ssh_cipher.h"
#include "fqterm_ssh_types.h"

namespace FQTerm {

class FQTermSSH1DES3: public FQTermSSHCipher {
 private:
  DES_cblock d_IV1;
  DES_cblock d_IV2;
  DES_cblock d_IV3;
  DES_key_schedule d_key1;
  DES_key_schedule d_key2;
  DES_key_schedule d_key3;
 public:
  FQTermSSH1DES3();

  virtual int blockSize() const;
  virtual int getKeySize() const;
  virtual int getIVSize() const;
  virtual void setIV(const u_char *data);
  virtual void setKey(const u_char *data);
  virtual void encrypt(const u_char *source, u_char *dest, int len);
  virtual void decrypt(const u_char *source, u_char *dest, int len);
};

class FQTermSSH2TripleDESCBC: public FQTermSSHCipher {
 private:
  unsigned char IV_[8];
  unsigned char key_[24];

  EVP_CIPHER_CTX *ctx_;

 public:
  FQTermSSH2TripleDESCBC();
  ~FQTermSSH2TripleDESCBC();

  virtual int blockSize() const;
  virtual int getKeySize() const;
  virtual int getIVSize() const;
  virtual void setIV(const u_char *data);
  virtual void setKey(const u_char *data);
  virtual void encrypt(const u_char *source, u_char *dest, int len);
  virtual void decrypt(const u_char *source, u_char *dest, int len);
};

}  // namespace FQTerm

#endif  // FQTERM_SSH_DES_H
