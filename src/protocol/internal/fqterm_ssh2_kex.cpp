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

#include <vector>
#include <string>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/des.h>
#include <openssl/objects.h>
#include <openssl/evp.h>

#include "fqterm_ssh2_kex.h"
#include "fqterm_trace.h"
#include "ssh_pubkey_crypto.h"
#include "ssh_cipher.h"

namespace FQTerm {

FQTermSSH2Kex::FQTermSSH2Kex(const char *V_C, const char *V_S) 
    : FQTermSSHKex(V_C, V_S) {
  is_first_kex_ = true;
  kex_state_ = FQTermSSH2Kex::BEFORE_KEXINIT;

  I_C_len_ = 0;
  I_C_ = NULL;
  I_S_len_ = 0;
  I_S_ = NULL;

  bn_x_ = BN_new();
  bn_e_ = BN_new();
  ctx_ = BN_CTX_new();

  bn_K_ = BN_new();
  bn_f_ = BN_new();

  session_id_ = NULL;
}

FQTermSSH2Kex::~FQTermSSH2Kex() {
  delete[] I_C_;
  delete[] I_S_;

  BN_clear_free(bn_x_);
  BN_clear_free(bn_e_);
  BN_CTX_free(ctx_);

  BN_clear_free(bn_K_);
  BN_clear_free(bn_f_);

  delete[] session_id_;
}

void FQTermSSH2Kex::initKex(FQTermSSHPacketReceiver *packetReceiver, 
                            FQTermSSHPacketSender	*packetSender) {
  packet_receiver_ = packetReceiver;
  packet_sender_ = packetSender;
  packet_receiver_->disconnect(this);
  FQ_VERIFY(connect(packet_receiver_, SIGNAL(packetAvaliable(int)),
                    this, SLOT(handlePacket(int))));
  kex_state_ = FQTermSSH2Kex::BEFORE_KEXINIT;
  emit reKex();
}

void FQTermSSH2Kex::handlePacket(int type)
{
	switch (kex_state_) {
	case FQTermSSH2Kex::BEFORE_KEXINIT:
		if (!negotiateAlgorithms())
			return;
		exchangeKey();
		kex_state_ = FQTermSSH2Kex::WAIT_REPLY;
		break;
	case FQTermSSH2Kex::WAIT_REPLY:
		if (verifyKey()) {
			sendNewKeys();
			kex_state_ = FQTermSSH2Kex::SESSIONKEY_SENT;
		} else {
			emit kexError(tr("Key exchange failed!"));
		}
		break;
	case FQTermSSH2Kex::SESSIONKEY_SENT:
		if (changeKeyAlg()) {
			kex_state_ = FQTermSSH2Kex::KEYEX_OK;
			emit kexOK();
		}
		break;
	case FQTermSSH2Kex::KEYEX_OK:
		// TODO: how about Key Re-Exchange (see RFC 4253, 9. Key Re-Exchange)
		break;
	}
}

bool FQTermSSH2Kex::negotiateAlgorithms() {
  FQ_FUNC_TRACE("ssh2kex", 10);

  if (packet_receiver_->packetType() != SSH2_MSG_KEXINIT) {
    emit kexError(tr("startKex: First packet is not SSH_MSG_KEXINIT"));
    return false;
  }

  // 0. Backup the payload of this server packet.
  I_S_len_ = packet_receiver_->packetDataLen() + 1;  // add 1 bytes for packet type.
  delete[] I_S_;
  I_S_ = new char[I_S_len_];
  I_S_[0] = SSH2_MSG_KEXINIT;
  memcpy(I_S_ + 1, packet_receiver_->buffer_->data(), I_S_len_ - 1);

  // 1. Parse server kex init packet
  packet_receiver_->getRawData((char*)cookie_, 16);

  // select KEX algorithm
  size_t kl_len = packet_receiver_->getInt();
  char kex_algos[kl_len+1];
  packet_receiver_->getRawData(kex_algos, kl_len);
  kex_algos[kl_len] = '\0';
  NEW_DH new_dh = search_dh(kex_algos);
  if (new_dh==NULL) {
	  emit kexError(tr("No matching KEX algorithms!"));
	  return false;
  }
  this->dh = new_dh();

  // TODO: host key algorithms
  size_t hk_algo_len = packet_receiver_->getInt();
  char hk_algo[hk_algo_len+1];
  packet_receiver_->getRawData(hk_algo, hk_algo_len);
  hk_algo[hk_algo_len] = '\0';

  // encryption algo c2s
  size_t el_c2s_len = packet_receiver_->getInt();
  char el_c2s[el_c2s_len+1];
  packet_receiver_->getRawData(el_c2s, el_c2s_len);
  el_c2s[el_c2s_len] = '\0';
  NEW_CIPHER c2s = search_cipher(el_c2s);
  if (c2s==NULL) {
	  emit kexError(tr("No matching c2s cipher algorithms!"));
	  return false;
  }
  packet_sender_->cipher = c2s(1);

  // encryption algo s2c
  size_t el_s2c_len = packet_receiver_->getInt();
  char el_s2c[el_s2c_len+1];
  packet_receiver_->getRawData(el_s2c, el_s2c_len);
  el_s2c[el_s2c_len] = '\0';
  NEW_CIPHER s2c = search_cipher(el_s2c);
  if (s2c==NULL) {
	  emit kexError(tr("No matching s2c cipher algorithms!"));
	  return false;
  }
  packet_receiver_->cipher = s2c(0);

  std::vector<char> name_lists;
  for (int i = 4; i < 10; ++i) {
    int name_lists_len = packet_receiver_->getInt();
    if (name_lists_len > 0) {
      name_lists.resize(name_lists_len);
      packet_receiver_->getRawData(&name_lists[0], name_lists_len);
      FQ_TRACE("ssh2kex", 5) << "Algorithms: " << QString::fromLatin1(&name_lists[0], name_lists_len);
    } else {
      FQ_TRACE("ssh2kex", 5) << "None Algorithms";
    }
  }

  bool first_kex_packet_follows = packet_receiver_->getByte();
  FQ_TRACE("ssh2kex", 5) << "first_kex_packet_follows: " << first_kex_packet_follows;

  packet_receiver_->consume(4);

  // 2. compose a kex init packet.
  packet_sender_->startPacket(SSH2_MSG_KEXINIT);
  packet_sender_->putRawData((const char*)cookie_, 16);    // FIXME: generate new cookie_;
  packet_sender_->putString(all_dh_list);
  packet_sender_->putString("ssh-rsa");
  packet_sender_->putString(all_ciphers_list);
  packet_sender_->putString(all_ciphers_list);
  packet_sender_->putString("hmac-sha1");
  packet_sender_->putString("hmac-sha1");
  packet_sender_->putString("none");
  packet_sender_->putString("none");
  packet_sender_->putString("");
  packet_sender_->putString("");

  packet_sender_->putByte(false);
  packet_sender_->putInt(0);

  // 3. backup the payload of this client packet.
  I_C_len_ = packet_sender_->buffer_->len();
  delete[] I_C_;
  I_C_ = new char[I_C_len_];
  memcpy(I_C_, packet_sender_->buffer_->data(), I_C_len_);

  // 4. send packet to server
  packet_sender_->write();

  return true;
}

void FQTermSSH2Kex::exchangeKey() {
  BN_pseudo_rand_range(bn_x_, dh->p);
  BN_mod_exp(bn_e_, dh->g, bn_x_, dh->p, ctx_);

  packet_sender_->startPacket(SSH2_MSG_KEXDH_INIT);
  packet_sender_->putBN2(bn_e_);
  packet_sender_->write();
}

static RSA *CreateRSAContext(unsigned char *host_key, int len);

bool FQTermSSH2Kex::verifyKey() {
  if (packet_receiver_->packetType() != SSH2_MSG_KEXDH_REPLY) {
    emit kexError(tr("Expect a SSH_MSG_KEXDH_REPLY packet"));
    return false;
  }

  // Extract data

  int K_S_len = -1;
  unsigned char *K_S = (unsigned char *)packet_receiver_->getString(&K_S_len);

  packet_receiver_->getBN2(bn_f_);

  int s_len = -1;
  unsigned char *s = (unsigned char *)packet_receiver_->getString(&s_len);

  BN_mod_exp(bn_K_, bn_f_, bn_x_, dh->p, ctx_);

  FQTermSSHBuffer *buffer = packet_sender_->output_buffer_;

  buffer->clear();
  buffer->putString(V_C_);
  buffer->putString(V_S_);
  buffer->putString(I_C_, I_C_len_);
  buffer->putString(I_S_, I_S_len_);
  buffer->putString((char *)K_S, K_S_len);
  buffer->putSSH2BN(bn_e_);
  buffer->putSSH2BN(bn_f_);
  buffer->putSSH2BN(bn_K_);

  ssh_dh_hash(dh, buffer->data(), H_, buffer->len());

  // Start verify
  // ssh-rsa specifies SHA-1 hashing
  unsigned char s_H[SHA_DIGEST_LENGTH];
  SHA1(H_, dh->digest.hashlen, s_H);

  // Ignore the first 15 bytes of the signature of H sent from server:
  // algorithm_name_length[4], algorithm_name[7]("ssh-rsa") and signature_length[4].
  RSA *rsactx = CreateRSAContext(K_S, K_S_len);
  int sig_len = s_len - 15;
  unsigned char *sig = s + 15;
  int res = RSA_verify(NID_sha1, s_H, SHA_DIGEST_LENGTH,
                       sig, sig_len, rsactx);

  RSA_free(rsactx);

  delete [] K_S;
  delete [] s;

  return res == 1;
}

static RSA *CreateRSAContext(unsigned char *host_key, int len) {
  FQTermSSHBuffer buffer(len);

  buffer.putRawData((char *)host_key, len);

  int algo_len = -1;
  unsigned char *algo = (unsigned char *)buffer.getString(&algo_len);

  FQ_VERIFY(std::string("ssh-rsa") == std::string((char *)algo));

  int e_len = -1;
  unsigned char *e = (unsigned char *)buffer.getString(&e_len);

  int n_len = -1;
  unsigned char *n = (unsigned char *)buffer.getString(&n_len);


  RSA *rsa = RSA_new();
  BIGNUM *rsa_e = BN_new();
  BIGNUM *rsa_n = BN_new();

  BN_bin2bn(e, e_len, rsa_e);
  BN_bin2bn(n, n_len, rsa_n);

#ifdef HAVE_OPAQUE_STRUCTS
  RSA_set0_key(rsa, rsa_n, rsa_e, NULL);
#else
  rsa->n = rsa_n;
  rsa->e = rsa_e;
#endif
  delete[] algo;
  delete[] e;
  delete[] n;

  return rsa;
}

void FQTermSSH2Kex::sendNewKeys(){
  packet_sender_->startPacket(SSH2_MSG_NEWKEYS);
  packet_sender_->write();
}

bool FQTermSSH2Kex::changeKeyAlg() {
  if (packet_receiver_->packetType() != SSH2_MSG_NEWKEYS) {
    emit kexError(tr("Expect a SSH_MSG_NEWKEYS packet"));
    return false;
  }

  if (session_id_ == NULL) {
	  session_id_ = new unsigned char[dh->digest.hashlen];
	  memcpy(session_id_, H_, dh->digest.hashlen);
  }

  packet_sender_->setMacType(FQTERM_SSH_HMAC_SHA1);
  packet_receiver_->setMacType(FQTERM_SSH_HMAC_SHA1);

  // From RFC 4253 section 7.2:
  // Initial IV client to server: HASH(K || H || "A" || session_id)
  // (Here K is encoded as mpint and "A" as byte and session_id as raw
  // data.  "A" means the single character A, ASCII 65).
  //
  // Initial IV server to client: HASH(K || H || "B" || session_id)
  //
  // Encryption key client to server: HASH(K || H || "C" || session_id)
  //
  // Encryption key server to client: HASH(K || H || "D" || session_id)
  //
  // Integrity key client to server: HASH(K || H || "E" || session_id)
  //
  // Integrity key server to client: HASH(K || H || "F" || session_id)

  int IV_c2s_len = packet_sender_->getIVSize();
  unsigned char *IV_c2s = computeKey(IV_c2s_len, 'A');

  int IV_s2c_len = packet_receiver_->getIVSize();
  unsigned char *IV_s2c = computeKey(IV_s2c_len, 'B');

  int key_c2s_len = packet_sender_->getKeySize();
  unsigned char *key_c2s = computeKey(key_c2s_len, 'C');

  int key_s2c_len = packet_receiver_->getKeySize();
  unsigned char *key_s2c = computeKey(key_s2c_len, 'D');

  int mac_key_c2s_len = packet_sender_->getMacKeySize();
  unsigned char *mac_key_c2s = computeKey(mac_key_c2s_len, 'E');

  int mac_key_s2c_len = packet_receiver_->getMacKeySize();
  unsigned char *mac_key_s2c = computeKey(mac_key_s2c_len, 'F');


  packet_sender_->startEncryption(key_c2s, IV_c2s);
  packet_sender_->startMac(mac_key_c2s);

  packet_receiver_->startEncryption(key_s2c, IV_s2c);
  packet_receiver_->startMac(mac_key_s2c);

  /* now key exchange ends */
  ssh_dh_free(dh);

  delete[] IV_c2s;
  delete[] IV_s2c;
  delete[] key_c2s;
  delete[] key_s2c;
  delete[] mac_key_c2s;
  delete[] mac_key_s2c;

  return true;
}

unsigned char *FQTermSSH2Kex::computeKey(int expected_len, char flag)
{
	unsigned char *key = new unsigned char[expected_len + SHA_DIGEST_LENGTH];

	int len = 0;

	EVP_MD_CTX *mdctx = dh->digest.mdctx;
	const EVP_MD *md = dh->digest.md;
	int hashlen = dh->digest.hashlen;

	FQTermSSHBuffer K(BN_num_bytes(bn_K_) + 5);
	K.putSSH2BN(bn_K_);

	while (len < expected_len) {
		EVP_DigestInit_ex(mdctx, md, NULL);
		EVP_DigestUpdate(mdctx, K.data(), K.len());
		EVP_DigestUpdate(mdctx, H_, hashlen);

		if (len == 0) {
			EVP_DigestUpdate(mdctx, &flag, 1);
			EVP_DigestUpdate(mdctx, session_id_, hashlen);
		} else {
			EVP_DigestUpdate(mdctx, key, len);
		}

		EVP_DigestFinal_ex(mdctx, key+len, NULL);
		len += SHA_DIGEST_LENGTH;
	}

	return key;
}

}  // namespace FQTerm

#include "fqterm_ssh2_kex.moc"
