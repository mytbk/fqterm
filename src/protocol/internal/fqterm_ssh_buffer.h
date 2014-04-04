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

#ifndef FQTERM_SSH_BUFFER_H
#define FQTERM_SSH_BUFFER_H

#include <openssl/bn.h>
#include <string.h>

#include "fqterm_ssh_types.h"

namespace FQTerm {

#define SSH_BUFFER_MAX 10000000

class FQTermSSHBuffer {
 private:
  u_char *buffer_;
  int offset_;
  int buffer_size_;
  int alloc_size_;

  void ensure(int len);
  void rebuffer();

 public:
  FQTermSSHBuffer(int size);
  ~FQTermSSHBuffer();
  
  u_char *data() const {
    return buffer_ + offset_;
  }

  int len() const {
    return buffer_size_;
  }

  void consume(int len);
  
  void clear();
  
  void putRawData(const char *data, int len);
  void getRawData(char *data, int len);
  
  void putSSH1BN(BIGNUM *bignum);
  void getSSH1BN(BIGNUM *bignum);

  void putSSH2BN(BIGNUM *bignum);
  void getSSH2BN(BIGNUM *bignum);
  
  void putInt(u_int data);
  u_int getInt();
  
  void putWord(u_short data);
  u_short getWord();
  
  void putByte(int data);
  u_char getByte();
  
  void putString(const char *str, int len = -1);
  void *getString(int *length = NULL);
};

}  // namespace FQTerm

#endif  //FQTERM_SSH_BUFFER
