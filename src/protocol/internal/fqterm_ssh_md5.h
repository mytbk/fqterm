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

#ifndef FQTERM_SSH_MD5_H
#define FQTERM_SSH_MD5_H

#include <stdio.h>

#include <openssl/md5.h>

#include "fqterm_ssh_types.h"
#include "fqterm_ssh_hash.h"

namespace FQTerm {

class FQTermSSHMD5: public FQTermSSHHash {
protected:
  MD5_CTX d_md5;
public:
  FQTermSSHMD5()
    : FQTermSSHHash() {
    d_digestLength = 16;
    MD5_Init(&d_md5);
  }
  virtual ~FQTermSSHMD5() {}
  
  void update(u_char *data, int len);
  void final(u_char *digest);
};

}  // namespace FQTerm

#endif //FQTERM_SSH_MD5_H
