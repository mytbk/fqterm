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

#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "fqterm_trace.h"
#include "fqterm_ssh_buffer.h"
#include "fqterm_serialization.h"

namespace FQTerm {
//==============================================================================
// FQTermSSHBuffer
//==============================================================================

FQTermSSHBuffer::FQTermSSHBuffer(int size) {
  alloc_size_ = size;
  buffer_size_ = 0;
  offset_ = 0;
  buffer_ = new u_char[alloc_size_];
}

FQTermSSHBuffer::~FQTermSSHBuffer() {
  delete [] buffer_;
  offset_ = 0;
  buffer_size_ = 0;
  alloc_size_ = 0;
}

void FQTermSSHBuffer::ensure(int len) {
  if (len <= (alloc_size_ - (offset_ + buffer_size_))) {
    return ;
  } else {
    alloc_size_ = buffer_size_ + len;
    rebuffer();
  }
}

void FQTermSSHBuffer::rebuffer() {
  u_char *newBuffer;
  newBuffer = new u_char[alloc_size_];
  memset(newBuffer, 0, alloc_size_);
  memcpy(newBuffer, buffer_ + offset_, buffer_size_);
  delete [] buffer_;
  buffer_ = newBuffer;
  offset_ = 0;
}

void FQTermSSHBuffer::clear() {
  memset(buffer_, 0, alloc_size_);
  offset_ = 0;
  buffer_size_ = 0;
}

void FQTermSSHBuffer::consume(int len) {
  if (len > buffer_size_) {
    len = buffer_size_;
  } 
  
  offset_ += len;
  buffer_size_ -= len;
}

void FQTermSSHBuffer::putRawData(const char *data, int len) {
  if (len < 0) {
    FQ_TRACE("sshbuffer", 0) << "Write data error.";
  }

  ensure(len);
  memcpy((buffer_ + offset_ + buffer_size_), data, len);
  buffer_size_ += len;
}

void FQTermSSHBuffer::getRawData(char *data, int len) {
  if (len <= buffer_size_ && len >= 0) {
    memcpy(data, buffer_ + offset_, len);
    consume(len);
  } else {
    FQ_TRACE("sshbuffer", 0) << "Read too many data: " << len << " bytes.";
  } 
}

//==============================================================================
// Store an BIGNUM in the buffer with a 2-byte msb first bit count, followed by
// (bits+7)/8 bytes of binary data, msb first.
//==============================================================================

void FQTermSSHBuffer::putSSH1BN(BIGNUM *bignum) {
  int bits = BN_num_bits(bignum);
  int bin_size = (bits + 7) / 8;
  u_char *buf = new u_char[bin_size];
  int oi;
  u_char msg[2];

  // Get the value of in binary
  oi = BN_bn2bin(bignum, buf);
  if (oi != bin_size) {
    FQ_TRACE("sshbuffer", 0) << "BN_bn2bin() failed: oi = " << oi
                             << " != bin_size." << bin_size;
  } 

  // Store the number of bits in the buffer in two bytes, msb first
  htonu16(msg, bits);
  putRawData((char*)msg, 2);
  // Store the binary data.
  putRawData((char*)buf, oi);

  memset(buf, 0, bin_size);
  delete [] buf;
}



void FQTermSSHBuffer::putSSH2BN(BIGNUM *bignum) {
  // FIXME: support negative number and add error handling.

  FQ_VERIFY(!BN_is_negative(bignum));  // currently we don't support negative big number.

  if (BN_is_zero(bignum)) {
    this->putInt(0);
  } else {
    u_int bytes = BN_num_bytes(bignum) + 1;

    FQ_VERIFY(bytes >= 2); // currently we don't support big numbers so small

    std::vector<u_char> buf(bytes);
    buf[0] = 0;

    int bin_len = BN_bn2bin(bignum, &buf[0] + 1);

    FQ_VERIFY(bin_len == (int)bytes - 1);
      
    u_int no_high_bit = (buf[1] & 0x80) ? 0 : 1;

    this->putInt(bytes - no_high_bit);
    this->putRawData((const char *)&buf[0] + no_high_bit, bytes - no_high_bit);
  }
}

//==============================================================================
// Retrieves a BIGNUM from the buffer.
//==============================================================================

void FQTermSSHBuffer::getSSH1BN(BIGNUM *bignum) {
  int bits, bytes;
  u_char buf[2];
  u_char *bin;

  // Get the number for bits.
  getRawData((char*)buf, 2);
  bits = ntohu16(buf);
  // Compute the number of binary bytes that follow.
  bytes = (bits + 7) / 8;
  if (bytes > 8 *1024) {
    FQ_TRACE("sshbuffer", 0) << "Can't handle BN of size " << bytes;
    return ;
  }
  if (len() < bytes) {
    FQ_TRACE("sshbuffer", 0) << "The input buffer is too small.";
    return ;
  }
  bin = data();
  BN_bin2bn(bin, bytes, bignum);
  consume(bytes);
}

void FQTermSSHBuffer::getSSH2BN(BIGNUM *bignum) {
  // FIXME: support negative numbers and error handling
  int len;
  unsigned char *hex_data = (unsigned char *)getString(&len);

  FQ_VERIFY(!(len > 0 && (hex_data[0] & 0x80)));  // don't support negative numbers.

  FQ_VERIFY(len < 10240);  // don't support so large numbers.

  BIGNUM *res = BN_bin2bn(hex_data, len, bignum);

  FQ_VERIFY(res != NULL);

  delete hex_data;
}

u_short FQTermSSHBuffer::getWord() {
  u_char buf[2];
  u_short data;

  getRawData((char*)buf, 2);
  data = ntohu16(buf);
  return data;
}

void FQTermSSHBuffer::putWord(u_short data) {
  u_char buf[2];

  htonu16(buf, data);
  putRawData((char*)buf, 2);
}

u_int FQTermSSHBuffer::getInt() {
  u_char buf[4];
  u_int data;
  getRawData((char*)buf, 4);
  data = ntohu32(buf);
  return data;
}

void FQTermSSHBuffer::putInt(u_int data) {
  u_char buf[4];

  htonu32(buf, data);
  putRawData((char*)buf, 4);
}

//==============================================================================
// Return a character from the buffer (0-255).
//==============================================================================

u_char FQTermSSHBuffer::getByte() {
  u_char ch;

  getRawData((char*) &ch, 1);
  return ch;
}

//==============================================================================
// Stores a character in the buffer.
//==============================================================================

void FQTermSSHBuffer::putByte(int data) {
  u_char ch = data;

  putRawData((char*) &ch, 1);
}

//==============================================================================
// Stores an arbitrary binary string in the buffer.
//==============================================================================

void FQTermSSHBuffer::putString(const char *str, int len) {
  if (str == NULL) {
    FQ_TRACE("sshbuffer", 0) << "Can't put a null pointer string.";
    return ;
  }

  if (len < 0) {
    len = strlen(str);
  }

  putInt(len);
  putRawData(str, len);
}

//==============================================================================
// Return an arbitrary binary string from the buffer. The string cannot be 
// longer than 256k. The returned value points to memory allocated with new;
// It is the responsibility of the calling function to free the data. 
//==============================================================================

void *FQTermSSHBuffer::getString(int *length) {
  u_char *data;
  u_int len;

  // Get the length.
  len = getInt();
  if ((long)len > buffer_size_) {
    FQ_TRACE("sshbuffer", 0)
        << "String length " << len
        << " is greater than buffer size " << buffer_size_;
    return 0;
  }
  // Allocate space for the string. Add one byte for a null character.
  data = new u_char[len + 1];
  // Get the string.
  getRawData((char*)data, len);
  // Append a null character to make processing easier.
  data[len] = 0;
  if (length != NULL) {
    *length = len;
  } 
  // return the length of the string.
  return data;
}

}  // namespace FQTerm
