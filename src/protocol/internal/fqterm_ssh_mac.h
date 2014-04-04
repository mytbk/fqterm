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

#ifndef FQTERM_SSH_MAC_H
#define FQTERM_SSH_MAC_H

#include <openssl/sha.h>

namespace FQTerm {

class FQTermSSHMac {
public:
  FQTermSSHMac() {}
  virtual ~FQTermSSHMac() {}
  virtual void setKey(const unsigned char *key) = 0;
  virtual int keySize() const = 0;
  virtual int digestSize() const = 0;
  virtual void getDigest(const unsigned char *data, int len, unsigned char *digest) const = 0;
};

const int FQTERM_SSH_MAC_NONE = 0;
const int FQTERM_SSH_HMAC_SHA1 = 1;

class FQTermSSHSHA1: public FQTermSSHMac {
  unsigned char key_[SHA_DIGEST_LENGTH];

public:
  virtual void setKey(const unsigned char *key);
  virtual int keySize() const;
  virtual int digestSize() const;
  virtual void getDigest(const unsigned char *data, int len, unsigned char *digest) const;
};


}  // namespace FQTerm

#endif  // FQTERM_SSH_MAC_H
