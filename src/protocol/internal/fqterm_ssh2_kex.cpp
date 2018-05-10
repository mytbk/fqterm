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
  K_S_ = NULL;

  sess.session_id = NULL;
}

FQTermSSH2Kex::~FQTermSSH2Kex() {
  delete[] I_C_;
  delete[] I_S_;
  if (K_S_)
    delete [] K_S_;

  if (sess.session_id != NULL)
    delete[] sess.session_id;
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

  /*
   * RFC 4253 section 7: kex begins by the following packet
   * byte         SSH_MSG_KEXINIT
   * byte[16]     cookie (random bytes)
   * name-list    kex_algorithms
   * name-list    server_host_key_algorithms
   * name-list    encryption_algorithms_client_to_server
   * name-list    encryption_algorithms_server_to_client
   * name-list    mac_algorithms_client_to_server
   * name-list    mac_algorithms_server_to_client
   * name-list    compression_algorithms_client_to_server
   * name-list    compression_algorithms_server_to_client
   * name-list    languages_client_to_server
   * name-list    languages_server_to_client
   * boolean      first_kex_packet_follows
   * uint32       0 (reserved for future extension)
   */

  if (packet_receiver_->packetType() != SSH2_MSG_KEXINIT) {
    emit kexError(tr("startKex: First packet is not SSH_MSG_KEXINIT"));
    return false;
  }

  // 0. Backup the payload of this server packet.
  I_S_len_ = packet_receiver_->packetDataLen() + 1;  // add 1 bytes for packet type.
  delete[] I_S_;
  I_S_ = new char[I_S_len_];
  I_S_[0] = SSH2_MSG_KEXINIT;
  memcpy(I_S_ + 1, buffer_data(&packet_receiver_->recvbuf), I_S_len_ - 1);

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
  sess.dh = new_dh();

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

  // mac algo c2s
  size_t m_c2s_len = packet_receiver_->getInt();
  char m_c2s[m_c2s_len+1];
  packet_receiver_->getRawData(m_c2s, m_c2s_len);
  m_c2s[m_c2s_len] = '\0';
  const struct ssh_mac_t * mac_c2s = search_mac(m_c2s);
  if (mac_c2s == NULL) {
	  emit kexError(tr("No matching c2s MAC algorithms!"));
	  return false;
  }
  packet_sender_->mac = mac_c2s->new_mac(mac_c2s);

  // mac algo s2c
  size_t m_s2c_len = packet_receiver_->getInt();
  char m_s2c[m_s2c_len+1];
  packet_receiver_->getRawData(m_s2c, m_s2c_len);
  m_s2c[m_s2c_len] = '\0';
  const struct ssh_mac_t * mac_s2c = search_mac(m_s2c);
  if (mac_s2c == NULL) {
	  emit kexError(tr("No matching s2c MAC algorithms!"));
	  return false;
  }
  packet_receiver_->mac = mac_s2c->new_mac(mac_s2c);

  std::vector<char> name_lists;
  for (int i = 6; i < 10; ++i) {
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
  packet_sender_->putRawData((const uint8_t*)cookie_, 16);    // FIXME: generate new cookie_;
  packet_sender_->putString(all_dh_list);
  packet_sender_->putString("ssh-rsa");
  packet_sender_->putString(all_ciphers_list);
  packet_sender_->putString(all_ciphers_list);
  packet_sender_->putString(all_macs_list);
  packet_sender_->putString(all_macs_list);
  packet_sender_->putString("none");
  packet_sender_->putString("none");
  packet_sender_->putString("");
  packet_sender_->putString("");

  packet_sender_->putByte(false);
  packet_sender_->putInt(0);

  // 3. backup the payload of this client packet.
  I_C_len_ = buffer_len(&packet_sender_->orig_data);
  delete[] I_C_;
  I_C_ = new char[I_C_len_];
  memcpy(I_C_, buffer_data(&packet_sender_->orig_data), I_C_len_);

  // 4. send packet to server
  packet_sender_->write();

  return true;
}

/* RFC 4253 section 8:
 * client generate x and compute e=g^x
 * server generate y and compute f=g^y
 *
 * client sends:
 * byte      SSH_MSG_KEXDH_INIT
 * mpint     e
 * server sends:
 * byte      SSH_MSG_KEXDH_REPLY
 * string    server public host key and certificates (K_S)
 * mpint     f
 * string    signature of H
 */

void FQTermSSH2Kex::exchangeKey()
{
	packet_sender_->startPacket(SSH2_MSG_KEXDH_INIT);
	packet_sender_->putRawData(sess.dh->mpint_e, sess.dh->e_len);
	packet_sender_->write();
}

static RSA *CreateRSAContext(unsigned char *host_key, int len);

bool FQTermSSH2Kex::verifyKey() {
  if (packet_receiver_->packetType() != SSH2_MSG_KEXDH_REPLY) {
    emit kexError(tr("Expect a SSH_MSG_KEXDH_REPLY packet"));
    return false;
  }


  if (K_S_)
    delete [] K_S_;
  K_S_ = (char*)packet_receiver_->getString(&K_S_len_);

  int mpint_f_len;
  unsigned char *mpint_f = (unsigned char *)packet_receiver_->getString(&mpint_f_len);
  if (ssh_dh_compute_secret(sess.dh, mpint_f, mpint_f_len) < 0) {
    emit kexError(tr("Error when computing shared secret"));
    delete mpint_f;
    return false;
  }

  int s_len = -1;
  unsigned char *s = (unsigned char *)packet_receiver_->getString(&s_len);

  buffer vbuf;
  buffer_init(&vbuf);
  buffer_append_string(&vbuf, V_C_, strlen(V_C_));
  buffer_append_string(&vbuf, V_S_, strlen(V_S_));
  buffer_append_string(&vbuf, I_C_, I_C_len_);
  buffer_append_string(&vbuf, I_S_, I_S_len_);
  buffer_append_string(&vbuf, K_S_, K_S_len_);
  buffer_append(&vbuf, sess.dh->mpint_e, sess.dh->e_len);
  buffer_append_string(&vbuf, (const char*)mpint_f, mpint_f_len);
  buffer_append(&vbuf, sess.dh->secret, sess.dh->secret_len);

  ssh_dh_hash(sess.dh, buffer_data(&vbuf), sess.H, buffer_len(&vbuf));

  buffer_deinit(&vbuf);

  // Start verify
  // ssh-rsa specifies SHA-1 hashing
  unsigned char s_H[SHA_DIGEST_LENGTH];
  SHA1(sess.H, sess.dh->digest.hashlen, s_H);

  // Ignore the first 15 bytes of the signature of H sent from server:
  // algorithm_name_length[4], algorithm_name[7]("ssh-rsa") and signature_length[4].
  RSA *rsactx = CreateRSAContext((unsigned char*)K_S_, K_S_len_);
  if (rsactx == NULL) {
	  emit kexError("Fail to get the RSA key!");
	  return false;
  }
  int sig_len = s_len - 15;
  unsigned char *sig = s + 15;
  int res = RSA_verify(NID_sha1, s_H, SHA_DIGEST_LENGTH,
                       sig, sig_len, rsactx);

  RSA_free(rsactx);

  delete [] mpint_f;
  delete [] s;

  return res == 1;
}

static RSA *CreateRSAContext(unsigned char *hostkey, int len)
{
	int algo_len, e_len, n_len;
	RSA *rsa = RSA_new();
	BIGNUM *rsa_e = BN_new();
	BIGNUM *rsa_n = BN_new();

	if (len >= 4)
		algo_len = be32toh(*(uint32_t*)hostkey);
	else
		goto fail;
	hostkey += 4;
	len -= 4;

	if (!(len >= 7 && algo_len == 7 && memcmp(hostkey, "ssh-rsa", 7) == 0))
		goto fail;
	hostkey += 7;
	len -= 7;

	if (len >= 4)
		e_len = be32toh(*(uint32_t*)hostkey);
	else
		goto fail;
	if (len >= 4+e_len)
		BN_mpi2bn(hostkey, 4+e_len, rsa_e);
	else
		goto fail;
	hostkey += 4 + e_len;
	len -= 4 + e_len;

	if (len >= 4)
		n_len = be32toh(*(uint32_t*)hostkey);
	else
		goto fail;
	if (len >= 4+n_len)
		BN_mpi2bn(hostkey, 4+n_len, rsa_n);
	else
		goto fail;
	hostkey += 4 + n_len;
	len -= 4 + n_len;

#ifdef HAVE_OPAQUE_STRUCTS
	RSA_set0_key(rsa, rsa_n, rsa_e, NULL);
#else
	rsa->n = rsa_n;
	rsa->e = rsa_e;
#endif

	return rsa;
fail:
	BN_clear_free(rsa_e);
	BN_clear_free(rsa_n);
	RSA_free(rsa);
	return NULL;
}

void FQTermSSH2Kex::sendNewKeys(){
  packet_sender_->startPacket(SSH2_MSG_NEWKEYS);
  packet_sender_->write();
}

bool FQTermSSH2Kex::changeKeyAlg() {
	unsigned char IV_c2s[128], IV_s2c[128], key_c2s[128], key_s2c[128], mac_c2s[128], mac_s2c[128];

  if (packet_receiver_->packetType() != SSH2_MSG_NEWKEYS) {
    emit kexError(tr("Expect a SSH_MSG_NEWKEYS packet"));
    return false;
  }

  if (sess.session_id == NULL) {
	  sess.session_id = new unsigned char[sess.dh->digest.hashlen];
	  memcpy(sess.session_id, sess.H, sess.dh->digest.hashlen);
  }

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
  computeKey(&sess, IV_c2s_len, 'A', IV_c2s);

  int IV_s2c_len = packet_receiver_->getIVSize();
  computeKey(&sess, IV_s2c_len, 'B', IV_s2c);

  int key_c2s_len = packet_sender_->getKeySize();
  computeKey(&sess, key_c2s_len, 'C', key_c2s);

  int key_s2c_len = packet_receiver_->getKeySize();
  computeKey(&sess, key_s2c_len, 'D', key_s2c);

  int mac_key_c2s_len = packet_sender_->getMacKeySize();
  computeKey(&sess, mac_key_c2s_len, 'E', mac_c2s);

  int mac_key_s2c_len = packet_receiver_->getMacKeySize();
  computeKey(&sess, mac_key_s2c_len, 'F', mac_s2c);

  packet_sender_->startEncryption(key_c2s, IV_c2s);
  packet_sender_->startMac(mac_c2s);

  packet_receiver_->startEncryption(key_s2c, IV_s2c);
  packet_receiver_->startMac(mac_s2c);

  /* now key exchange ends */
  ssh_dh_free(sess.dh);

  return true;
}

}  // namespace FQTerm

#include "fqterm_ssh2_kex.moc"
