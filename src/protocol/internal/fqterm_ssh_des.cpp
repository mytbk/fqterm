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

#include <memory.h>
#include <openssl/des.h>
#include <openssl/objects.h>
#include <openssl/evp.h>

#include "fqterm_ssh_const.h"
#include "fqterm_ssh_des.h"
#include "fqterm_trace.h"

namespace FQTerm {

#define DES_ENCRYPT 1
#define DES_DECRYPT 0

//==============================================================================
//FQTermSSH1DES3
//==============================================================================

FQTermSSH1DES3::FQTermSSH1DES3() {
  d_name = "des3-cbc";

  memset(d_IV1, 0, sizeof(d_IV1));
  memset(d_IV2, 0, sizeof(d_IV2));
  memset(d_IV3, 0, sizeof(d_IV3));
}

int FQTermSSH1DES3::blockSize() const {
  return 8;
}

void FQTermSSH1DES3::decrypt(const u_char *source, u_char *dest, int len) {
  DES_ncbc_encrypt(source, dest, len, &d_key3, &d_IV3, DES_DECRYPT);
  DES_ncbc_encrypt(dest, dest, len, &d_key2, &d_IV2, DES_ENCRYPT);
  DES_ncbc_encrypt(dest, dest, len, &d_key1, &d_IV1, DES_DECRYPT);
}

void FQTermSSH1DES3::encrypt(const u_char *source, u_char *dest, int len) {
  DES_ncbc_encrypt(source, dest, len, &d_key1, &d_IV1, DES_ENCRYPT);
  DES_ncbc_encrypt(dest, dest, len, &d_key2, &d_IV2, DES_DECRYPT);
  DES_ncbc_encrypt(dest, dest, len, &d_key3, &d_IV3, DES_ENCRYPT);
}

int FQTermSSH1DES3::getKeySize() const {
  return 3*DES_KEY_SZ;
}

int FQTermSSH1DES3::getIVSize() const {
  return 0;
}

void FQTermSSH1DES3::setIV(const u_char *data) {
  memset(d_IV1, 0, sizeof(d_IV1));
  memset(d_IV2, 0, sizeof(d_IV2));
  memset(d_IV3, 0, sizeof(d_IV3));
}

void FQTermSSH1DES3::setKey(const u_char *data) {
  DES_cblock key;
  memset(key, 0, sizeof(key));
  memcpy(key, data, sizeof(key));
  DES_set_key(&key, &d_key1);
  data += 8;
  memset(key, 0, sizeof(key));
  memcpy(key, data, sizeof(key));
  DES_set_key(&key, &d_key2);
  data += 8;
  memset(key, 0, sizeof(key));
  memcpy(key, data, sizeof(key));
  DES_set_key(&key, &d_key3);
}


//==================================================
// TripleDES-CBC
//==================================================

FQTermSSH2TripleDESCBC::FQTermSSH2TripleDESCBC() {
  d_name = "des3-cbc";
  ctx_ = NULL;
}

FQTermSSH2TripleDESCBC::~FQTermSSH2TripleDESCBC() {
  if (ctx_ != NULL) {
    EVP_CIPHER_CTX_cleanup(ctx_);
    delete ctx_;
  }
}


int FQTermSSH2TripleDESCBC::blockSize() const {
  return 8;
}

int FQTermSSH2TripleDESCBC::getKeySize() const {
  return 24;  
}

int FQTermSSH2TripleDESCBC::getIVSize() const {
  return 8;
}

void FQTermSSH2TripleDESCBC::setIV(const u_char *data) {
  memcpy(IV_, data, getIVSize());
}

void FQTermSSH2TripleDESCBC::setKey(const u_char *data) {
  memcpy(key_, data, getKeySize());
}

void FQTermSSH2TripleDESCBC::encrypt(const u_char *source, u_char *dest, int len) {
  FQ_TRACE("3DES_CBC", 9) << "Start encrypting";  
  FQ_TRACE("3DES_CBC", 9) << "data len:" << len;
  FQ_TRACE("3DES_CBC", 9) << "Source: \n" << dumpHexString << std::string((char *)source, len);

  int ret = 0;
  if (ctx_ == NULL) {
    // Lazy initialization.
    ctx_ = new EVP_CIPHER_CTX;
    EVP_CIPHER_CTX_init(ctx_);
    ret = EVP_CipherInit(ctx_, EVP_des_ede3_cbc(), key_, IV_, 1);
    FQ_VERIFY(ret == 1);
  }

  ret = EVP_Cipher(ctx_, dest, source, len);

  FQ_VERIFY(ret == 1);

  FQ_TRACE("3DES_CBC", 9) << "Dest: \n" << dumpHexString << std::string((char *)dest, len);

  FQ_TRACE("3DES_CBC", 9) << "IV:\n" << dumpHexString << std::string((char *)IV_, getIVSize());
  FQ_TRACE("3DES_CBC", 9) << "key:\n" << dumpHexString << std::string((char *)key_, getKeySize());

}

void FQTermSSH2TripleDESCBC::decrypt(const u_char *source, u_char *dest, int len) {
  FQ_TRACE("3DES_CBC", 9) << "Start dencrypting";  
  FQ_TRACE("3DES_CBC", 9) << "data len:" << len;
  FQ_TRACE("3DES_CBC", 9) << "Source: \n" << dumpHexString << std::string((char *)source, len);

  int ret = 0;
  if (ctx_ == NULL) {
    // Lazy initialization.
    ctx_ = new EVP_CIPHER_CTX;
    EVP_CIPHER_CTX_init(ctx_);
    ret = EVP_CipherInit(ctx_, EVP_des_ede3_cbc(), key_, IV_, 0);
    FQ_VERIFY(ret == 1);
  }

  ret = EVP_Cipher(ctx_, dest, source, len);

  FQ_VERIFY(ret == 1);

  FQ_TRACE("3DES_CBC", 9) << "Dest: \n" << dumpHexString << std::string((char *)dest, len);

  FQ_TRACE("3DES_CBC", 9) << "IV:\n" << dumpHexString << std::string((char *)IV_, getIVSize());
  FQ_TRACE("3DES_CBC", 9) << "key:\n" << dumpHexString << std::string((char *)key_, getKeySize());
}


}  // namespace FQTerm
