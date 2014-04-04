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

#ifndef FQTERM_SSH_CIPHER_H
#define FQTERM_SSH_CIPHER_H

#include <stdlib.h>

#include "fqterm_ssh_types.h"
#include "fqterm_ssh_const.h"

namespace FQTerm {

class FQTermSSHCipher {
protected:
  const char *d_name;

public:
  FQTermSSHCipher() {
    d_name = NULL;
  }
  
  virtual ~FQTermSSHCipher(){}
  
  const char *name() const {
    return d_name;
  }

  virtual int blockSize() const = 0;
  virtual int getKeySize() const = 0;
  virtual int getIVSize() const = 0;
  
  virtual void setIV(const u_char *data) = 0;
  virtual void setKey(const u_char *data) = 0;
  virtual void encrypt(const u_char *source, u_char *dest, int len) = 0;
  virtual void decrypt(const u_char *source, u_char *dest, int len) = 0;
};

}  // namespace FQTerm

#endif  // FQTERM_SSH_CIPHER_H
