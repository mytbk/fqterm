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

#include <QGlobalStatic>

#include "fqterm_trace.h"
#include "fqterm_ssh_types.h"
#include "fqterm_ssh_rsa.h"

namespace FQTerm {

FQTermSSHRSA::FQTermSSHRSA() {
  d_rsa = RSA_new();
  d_rsa->n = BN_new();
  d_rsa->e = BN_new();
}

FQTermSSHRSA::~FQTermSSHRSA() {
  if (d_rsa != NULL) {
    RSA_free(d_rsa);
  } 
}

void FQTermSSHRSA::publicEncrypt(BIGNUM *out, BIGNUM *in) {
  u_char *inbuf,  *outbuf;
  int len, ilen, olen;
  
  if (BN_num_bits(d_rsa->e) < 2 || !BN_is_odd(d_rsa->e)) {
    FQ_VERIFY(false);  // public_encrypt() exponent too small or not odd.
  } 
  
  olen = BN_num_bytes(d_rsa->n);
  outbuf = new u_char[olen];
  
  ilen = BN_num_bytes(in);
  inbuf = new u_char[ilen];
  BN_bn2bin(in, inbuf);
  
  if ((len = RSA_public_encrypt(ilen, inbuf, outbuf, d_rsa, RSA_PKCS1_PADDING)) <= 0) { 
    //RSA_PKCS1_PADDING = 1
    FQ_VERIFY(false);  // "rsa_public_encrypt() failed.
  } 

  BN_bin2bn(outbuf, len, out);

  delete [] outbuf;
  delete [] inbuf;
}

}  // namespace FQTerm
