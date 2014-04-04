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

#ifndef FQTERM_TELNET_H
#define FQTERM_TELNET_H

#include <QAbstractSocket>
#include <QByteArray>

namespace FQTerm {

#ifndef u_char
#define u_char uchar
#endif

#define NTSTATES	 6	    // # of Telnet Socket-Input FSM States
#define NSSTATES	3		// # of Telnet Option Subnegotiation FSM States
#define NCHRS		256		// # of valid characters

//  decleration
class FQTermTelnet;
// actionFunc is a pointer, point to a FQTermTelnet's func
typedef int(FQTermTelnet:: *ptrActionFunc)(int c);

//fsm struct
struct fsm_trans {
  u_char ft_state; // current state
  short ft_char; // input character
  u_char ft_next; // next state
  ptrActionFunc ft_action; // action to take
};


/*------------------------------------------------------------------------------
 *	FQTermTelnet class definition
 *-------------------------------------------------------------------------------
 */
class FQTermSocket;

// Telnet connection, a wrapper of socket.
// It will translate raw NVT data from low level socket to ansi data,
// and then upper level application can read it.
// It also can send ascii data (0~127).
class FQTermTelnet: public QObject {
  Q_OBJECT;
 public:
  FQTermTelnet(const QString &termtype, int rows, int numColumns, int protocolType, int hostType, const
              char *sshuser = NULL, const char *sshpasswd = NULL);
  ~FQTermTelnet();

  void setProxy(int nProxyType,  //0-no proxy; 1-wingate; 2-sock4; 3-socks5
                bool bAuth,  // if authentation needed
                const QString &strProxyHost, quint16 uProxyPort, const QString &strProxyUsr,
                const QString &strProxyPwd);
  void connectHost(const QString &hostname, quint16 portnumber);

  // Read ansi data.
  int read(char *data, uint maxlen);

  // Write data raw data
  int write(const char *data, uint len);

  void close(); // User close the connection

  int raw_len();
  int read_raw(char *data, uint maxlen);

  bool readyForInput();


 signals:
  void readyRead(int, int); // There are datas to be read out
  void TelnetState(int); // The  state telnet, defined as TSXXXX in fqterm.h
  void requestUserPwd(QString *user, QString *pwd, bool *isOK);
  void errorMessage(QString);

 public slots:
  void windowSizeChanged(int, int);

 private slots:
  // Retrieve data from socket, translate NVT data to ansi data,
  // then notify data ready.
  void socketReadyRead();

  void connected();
  void showError(QAbstractSocket::SocketError);
  void hostFound();
  void delayCloseFinished();
  void closed();
 protected:
  //init structure fsm
  void init_telnet();
  void fsmbuild();
  void fsminit(u_char fsm[][NCHRS], struct fsm_trans ttab[], int nstates);

  //actions
  int tcdm(int);
  int recopt(int);
  int no_op(int);
  int do_echo(int);
  int do_notsup(int);
  int do_noga(int);
  int do_txbinary(int);
  int will_notsup(int);
  int will_txbinary(int);
  int will_termtype(int);
  int will_naws(int);
  int subopt(int);
  int subtermtype(int);
  int subend(int);
  int soputc(int);
  int ttputc(int);
  int tnabort(int);

  //utility functions
  int xputc_up(char);
  int xputs_up(char*);
  void putc_down(u_char);

signals:
  
  void onSSHAuthOK();

 private:
  // Boolean Flags
  char synching, doecho, sndbinary, rcvbinary;
  char noga;
  char naws;
  char server_sent_do_naws;
  u_char option_cmd; // has value WILL, WONT, DO, or DONT

  char termtype; // non-zero if received "DO TERMTYPE"
  char *term; // terminal name

  /* // TODO: BBS don't need control signals
  // Special keys - Terminal control characters
  // need work...
  static const char t_flushc=FQTERM_CTRL('S'); // Abort Output		i.e:(^S)
  static const char t_intrc=FQTERM_CTRL('C');  // Interrupt		i.e:(^C)
  static const char t_quitc=FQTERM_CTRL('\\'); // Quit			i.e:(^\)
  static const char sg_erase=FQTERM_CTRL('?'); // Erase a character	i.e:(^?)
  static const char sg_kill=FQTERM_CTRL('U');  // Kill a line		i.e:(^U)
  */

  // FSM stuffs
  static struct fsm_trans ttstab[];
  int ttstate;
  u_char ttfsm[NTSTATES][NCHRS];

  static struct fsm_trans substab[];
  int substate;
  u_char subfsm[NSSTATES][NCHRS];

  // socket stuffs
  FQTermSocket *socket;

  //Pointers to internal buffers
  //
  //             |-->from_socket-->process-->to_ansi-->|
  //  socket<--->                                      <---> ansi decode
  //             |<---to_socket<--process<--from_ansi--|
  //
  QByteArray from_socket, to_ansi, from_ansi, to_socket;
  uint rsize; // size of to_ansi buffer
  uint wsize; // size of to_socket buffer

  // for test
  int wx, wy;
  int done_naws;
  bool bConnected;
  int raw_size;
  int hostType_;
  int protocolType_;
};

}  // namespace FQTerm

#endif  // FQTERM_TELNET_H
