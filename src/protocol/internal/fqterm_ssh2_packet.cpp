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
#include "fqterm_ssh2_packet.h"
#include "fqterm_ssh_des.h"

#include "fqterm_serialization.h"
#include "crc32.h"

namespace FQTerm {
//==============================================================================
//FQTermSSH2PacketSender
//==============================================================================
// SSH2 Packet Structure:
//      uint32    packet_length
//      byte      padding_length
//      byte[n1]  payload; n1 = packet_length - padding_length - 1
//      byte[n2]  random padding; n2 = padding_length
//      byte[m]   mac (Message Authentication Code - MAC); m = mac_length
//==============================================================================

void FQTermSSH2PacketSender::makePacket() {
  FQ_TRACE("ssh2packet", 9) << "----------------------------Send "
                            << (is_encrypt_ ? "Encrypted": "plain")
                            << " Packet---->>>>>>>";

  // 0. compress
  if (is_compressed_) {
    FQ_VERIFY(false);
  }

  // 1. compute the padding length for padding.
  int non_padding_len = 4 + 1 + buffer_->len();

  int padding_block_len = 8;
  if (is_encrypt_ && cipher_->blockSize() > padding_block_len) {
    padding_block_len = cipher_->blockSize();
  }

  int padding_len = padding_block_len - (non_padding_len % padding_block_len);
  if (padding_len < 4) {
    padding_len += padding_block_len;
  }

  // 2. renew the output buffer.
  int total_len = non_padding_len + padding_len;
  if (is_mac_) {
    total_len += mac_->digestSize();
  }

  delete output_buffer_;
  output_buffer_ = new FQTermSSHBuffer(total_len);

  // 3. Fill the output buffer.
  int packet_len = 1 + buffer_->len() + padding_len;

  output_buffer_->putInt(packet_len);
  output_buffer_->putByte(padding_len);
  output_buffer_->putRawData((const char*)buffer_->data(), buffer_->len());

  u_int32_t rand_val = 0;
  for (int i = 0; i < padding_len; i++) {
    if (i % 4 == 0) {
      rand_val = rand();  // FIXME:  rand() doesn't range from 0 to 2^32.
    } 
    output_buffer_->putByte(rand_val & 0xff);
    rand_val >>= 8;
  }

  // 4. Add MAC on the entire unencrypted packet,
  // including two length fields, 'payload' and 'random padding'.
  if (is_mac_) {
    const unsigned char *packet = output_buffer_->data();
    int len = output_buffer_->len();

    FQTermSSHBuffer buffer(4 + len);
    buffer.putInt(sequence_no_);
    buffer.putRawData((const char *)packet, len);

    std::vector<u_char> digest(mac_->digestSize());
    mac_->getDigest(buffer.data(), buffer.len(), &digest[0]);

    FQ_TRACE("ssh2packet", 9) << "Making packets...";
    FQ_TRACE("ssh2packet", 9) << "Append MAC with sequence_no_" << sequence_no_; 
    FQ_TRACE("ssh2packet", 9) << "Compute MAC with " 
                              << buffer.len() << " bytes data:\n" 
                              << dumpHexString << std::string((char *)buffer.data(), buffer.len());
    FQ_TRACE("ssh2packet", 9) << "MAC data " 
                              << digest.size() << " bytes:\n" 
                              << dumpHexString << std::string((const char *)&digest[0], digest.size());
 
    output_buffer_->putRawData((const char *)&digest[0], digest.size());
  }

  if (is_compressed_) {
    FQ_VERIFY(false);
  }

  if (is_encrypt_) {
    // as RFC 4253:
    // When encryption is in effect, the packet length, padding
    // length, payload, and padding fields of each packet MUST be encrypted
    // with the given algorithm.

    u_char *data = output_buffer_->data();
    int len = output_buffer_->len() - mac_->digestSize();

    FQ_TRACE("ssh2packet", 9) << "An packet (without MAC) to be encrypted:" 
                              << len << " bytes:\n" 
                              << dumpHexString << std::string((const char *)data, len);

    cipher_->encrypt(data, data, len);

    FQ_TRACE("ssh2packet", 9) << "An encrypted packet (without MAC) made:" 
                              << len << " bytes:\n" 
                              << dumpHexString << std::string((const char *)data, len);
  } 

  ++sequence_no_;
}

void FQTermSSH2PacketSender::setEncryptionType(int cipherType) {
  cipher_type_ = cipherType;

  delete cipher_;
  cipher_ = NULL;

  switch (cipher_type_) {
    case SSH_CIPHER_3DES:
      cipher_ = new FQTermSSH2TripleDESCBC;
      break;
  }
}


//==============================================================================
//FQTermSSH2PacketReceiver
//==============================================================================
void FQTermSSH2PacketReceiver::parseData(FQTermSSHBuffer *input) {
  FQ_TRACE("ssh2packet", 9) << "----------------------------Receive "
                            << (is_decrypt_ ? "Encrypted": "plain")
                            << " Packet----<<<<<<<";
  while (input->len() > 0) {
    // 1. Check the ssh packet
    if (input->len() < 16
        || (is_decrypt_ && input->len() < cipher_->blockSize())
        || input->len() < last_expected_input_length_ 
        ) {
      FQ_TRACE("ssh2packet", 3)
          << "Got an incomplete packet. Wait for more data.";
      return ;
    }

    if (last_expected_input_length_ == 0) {
      if (is_decrypt_) {
        // decrypte the first block to get the packet_length field.
        cipher_->decrypt(input->data(), input->data(), cipher_->blockSize());
      }
    } else {
      // last_expected_input_length_ != 0
      // indicates an incomplete ssh2 packet received last time,
      // the first block of data is already decrypted at that time,
      // so it must not be decrypted again.
    }

    int packet_len = ntohu32(input->data());

    if (packet_len > SSH_BUFFER_MAX) {
      emit packetError(tr("parseData: packet too big"));
      return ;
    }

    int expected_input_len = 4 + packet_len + (is_mac_ ? mac_->digestSize() : 0);

    if (input->len()  < (long)expected_input_len) {
      FQ_TRACE("ssh2packet", 3)
          << "The packet is too small. Wait for more data.";
      last_expected_input_length_ = expected_input_len;    
      return ;
    } else {
      last_expected_input_length_ = 0;      
    }

    // 2. decrypte data.
    if (is_decrypt_) {
      // decrypte blocks left.
      unsigned char *tmp = input->data() + cipher_->blockSize();
      int left_len = expected_input_len - cipher_->blockSize() - mac_->digestSize();
      cipher_->decrypt(tmp, tmp, left_len);
    }

    // 3. check MAC
    if (is_mac_) {
      int digest_len = mac_->digestSize();
      std::vector<u_char> digest(digest_len);
      mac_->getDigest(input->data(), expected_input_len - digest_len, &digest[0]);

      u_char *received_digest = input->data() + expected_input_len - digest_len;

      if (memcmp(&digest[0], received_digest, digest_len) == 0) {
        FQ_TRACE("ssh2packet", 0) << "incorrect MAC.";
        return ;
      }
    }

    // 4. get every field of the ssh packet.
    packet_len = input->getInt();

    std::vector<u_char> data(packet_len);
    
    input->getRawData((char*)&data[0], packet_len);
    if (is_mac_) {
      input->consume(mac_->digestSize());
    }

    int padding_len = data[0];

    real_data_len_ = packet_len - 1 - padding_len;

    buffer_->clear();
    buffer_->putRawData((char*)&data[0] + 1, real_data_len_);

    FQ_TRACE("ssh2packet", 9) << "Receive " << real_data_len_ << " bytes payload:\n" 
                              << dumpHexString << std::string((char *)&data[0] + 1, real_data_len_);

    // 5. notify others a ssh packet is parsed successfully.
    packet_type_ = buffer_->getByte();
    real_data_len_ -= 1;
    emit packetAvaliable(packet_type_);

    ++sequence_no_;
  }
}

void FQTermSSH2PacketReceiver::setEncryptionType(int cipherType) {
  cipher_type_ = cipherType;

  delete cipher_;
  cipher_ = NULL;

  switch (cipher_type_) {
    case SSH_CIPHER_3DES:
      cipher_ = new FQTermSSH2TripleDESCBC;
      break;
  }
}

}  // namespace FQTerm
