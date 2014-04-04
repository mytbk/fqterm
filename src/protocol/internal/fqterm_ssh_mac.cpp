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

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include "fqterm_trace.h"
#include "fqterm_ssh_mac.h"
#include "fqterm_ssh_packet.h"

namespace FQTerm {

void FQTermSSHSHA1::setKey(const unsigned char *key) {
  memcpy(key_, key, keySize());
}

int FQTermSSHSHA1::keySize() const {
  return SHA_DIGEST_LENGTH;
}

int FQTermSSHSHA1::digestSize() const {
  return SHA_DIGEST_LENGTH;
}

void FQTermSSHSHA1::getDigest(const unsigned char *data, int len, unsigned char *digest) const {
  unsigned int tmp;
  HMAC(EVP_sha1(), key_, SHA_DIGEST_LENGTH, data, len, digest, &tmp);

  FQ_TRACE("SHA1", 9) << "Key: \n" << dumpHexString << std::string((char *)key_, SHA_DIGEST_LENGTH);
  FQ_TRACE("SHA1", 9) << "data len:" << len;
  FQ_TRACE("SHA1", 9) << "data:\n" << dumpHexString << std::string((char *)data, len);
  FQ_TRACE("SHA1", 9) << "digest len:" << tmp;
  FQ_TRACE("SHA1", 9) << "digest:\n" << dumpHexString << std::string((char *)digest, tmp);
}

}  // namespace FQTerm
