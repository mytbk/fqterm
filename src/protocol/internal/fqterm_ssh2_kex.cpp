// SPDX-License-Identifier: GPL-2.0-or-later

#include <vector>

#include "fqterm_ssh2_kex.h"
#include "fqterm_trace.h"
#include "ssh_cipher.h"
#include "ssh_rsa.h"

namespace FQTerm {

FQTermSSH2Kex::FQTermSSH2Kex(const char *V_C, const char *V_S)
    : FQTermSSHKex(V_C, V_S)
{
	is_first_kex_ = true;
	kex_state_ = FQTermSSH2Kex::BEFORE_KEXINIT;

	sess.I_C = NULL;
	sess.I_S = NULL;
	sess.K_S = NULL;
	sess.session_id = NULL;
	sess.V_C = this->V_C_;
	sess.V_S = this->V_S_;
}

FQTermSSH2Kex::~FQTermSSH2Kex()
{
	if (sess.I_C)
		delete[] sess.I_C;

	if (sess.I_S)
		delete[] sess.I_S;

	if (sess.K_S)
		free(sess.K_S);

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
  sess.I_S_len = packet_receiver_->packetDataLen() + 1;  // add 1 bytes for packet type.
  if (sess.I_S)
	  delete[] sess.I_S;
  sess.I_S = new char[sess.I_S_len];
  sess.I_S[0] = SSH2_MSG_KEXINIT;
  memcpy(sess.I_S + 1, buffer_data(&packet_receiver_->recvbuf), sess.I_S_len - 1);

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
  sess.I_C_len = buffer_len(&packet_sender_->orig_data);
  if (sess.I_C)
	  delete [] sess.I_C;
  sess.I_C = new char[sess.I_C_len];
  memcpy(sess.I_C, buffer_data(&packet_sender_->orig_data), sess.I_C_len);

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

bool FQTermSSH2Kex::verifyKey()
{
	if (packet_receiver_->packetType() != SSH2_MSG_KEXDH_REPLY) {
		emit kexError(tr("Expect a SSH_MSG_KEXDH_REPLY packet"));
		return false;
	}

	buffer *buf = &packet_receiver_->recvbuf;
	int res = verifyRSAKey(&sess, buffer_data(buf), buffer_len(buf));
	switch (res) {
		case 1:
			return true;
		case 0:
			return false;
		case -ESECRET:
			emit kexError(tr("Error computing secret!"));
			return false;
		case -ERSA:
			emit kexError(tr("Fail to get the RSA key!"));
			return false;
		default:
			emit kexError(tr("verifyKey: unknown error!"));
			return false;
	}
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
