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
#include "fqterm_ssh1_packet.h"
#include "fqterm_ssh_des.h"

#include "fqterm_serialization.h"
#include "crc32.h"

namespace FQTerm {
//==============================================================================
//FQTermSSH1PacketSender
//==============================================================================
//==============================================================================
//
// SSH1 Packet Structure:
//  --------------------------------------------------------------------------
//  | length | padding  | type  |                data                | crc32 |
//  --------------------------------------------------------------------------
//  | uint32 | 1-7bytes | uchar |                                    | 4bytes|
//  --------------------------------------------------------------------------
//  encrypt = padding + type + data + crc32
//  length =  type + data + crc32
//
//==============================================================================

void FQTermSSH1PacketSender::makePacket() {
  int len, padding, i;
  u_int32_t rand_val = 0;

  delete output_buffer_;

  len = buffer_->len() + 4; //CRC32
  padding = 8-(len % 8);

  output_buffer_ = new FQTermSSHBuffer(len + padding + 4); //pktlen and crc32
  output_buffer_->putInt(len);

  for (i = 0; i < padding; i++) {
    if (i % 4 == 0) {
      rand_val = rand();  // FIXME:  rand() doesn't range from 0 to 2^32.
    } 
    
    output_buffer_->putByte(rand_val &0xff);
    rand_val >>= 8;
  }

  output_buffer_->putRawData((const char*)buffer_->data(), buffer_->len());
  output_buffer_->putInt(ssh_crc32(output_buffer_->data() + 4, output_buffer_->len() - 4));

  if (is_encrypt_) {
    cipher_->encrypt(output_buffer_->data() + 4, output_buffer_->data() + 4, output_buffer_->len() - 4);
  } 

}

//==============================================================================
//FQTermSSH1PacketReceiver
//==============================================================================
void FQTermSSH1PacketReceiver::parseData(FQTermSSHBuffer *input) {
  u_int mycrc, gotcrc;
  u_char *buf = NULL;
  u_char *targetData = NULL;
  u_char *sourceData = NULL;

  // Get the length of the packet.
  while (input->len() > 0) {
    if (input->len() < 4) {
      FQ_TRACE("ssh1packet", 3) << "The packet is too small.";
      return ;
    }
    buf = input->data();
    real_data_len_ = ntohu32(buf);

    if (real_data_len_ > SSH_BUFFER_MAX) {
      emit packetError(tr("parseData: The packet is too big"));
      return ;
    }

    u_int total_len = (real_data_len_ + 8) &~7;
    u_int padding_len = total_len - real_data_len_;

    real_data_len_ -= 5;
    buffer_->clear();

    // Get the data of the packet.
    if (input->len() - 4 < (long)total_len) {
      FQ_TRACE("ssh1packet", 3) << "The packet is too small";
      return ;
    }

    real_data_len_ = input->getInt() - 5;
    targetData = new u_char[total_len];
    sourceData = new u_char[total_len];
    memset(targetData, 0, total_len);
    memset(sourceData, 0, total_len);

    input->getRawData((char*)sourceData, total_len);
    if (is_decrypt_) {
      cipher_->decrypt(sourceData, targetData, total_len);
    } else {
      memcpy(targetData, sourceData, total_len);
    } 
    
    buffer_->putRawData((char*)targetData, total_len);

    // Check the crc32.
    buf = buffer_->data() + total_len - 4;
    mycrc = ntohu32(buf);
    gotcrc = ssh_crc32(buffer_->data(), total_len - 4);

    if (mycrc != gotcrc) {
      emit packetError(tr("parseData: bad CRC32"));
      break;
    }

    // Drop the padding.
    buffer_->consume(padding_len);

    packet_type_ = buffer_->getByte();

    emit packetAvaliable(packet_type_);

    delete [] sourceData;
    delete [] targetData;
  }
}

}  // namespace FQTerm
