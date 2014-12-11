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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <QAbstractSocket>

#include "fqterm.h"
#include "fqterm_telnet.h"
#include "fqterm_socket.h"
#include "fqterm_telnet_socket.h"
#include "fqterm_local_socket.h"

#ifndef _NO_SSH_COMPILED
#include "fqterm_ssh_socket.h"
#endif

/* TELNET Command Codes: */
/* Hints: These NVT control characters are sent from client to server, So
   the client side will not receive these commands */
#define TCSB		(u_char)250	/* Start Subnegotiation		*/
#define TCSE		(u_char)240	/* End Of Subnegotiation	*/
#define TCNOP		(u_char)241	/* No Operation			*/
#define TCDM		(u_char)242	/* Data Mark (for Sync)		*/
#define TCBRK		(u_char)243	/* NVT Character BRK		*/
#define TCIP		(u_char)244	/* Interrupt Process		*/
#define TCAO		(u_char)245	/* Abort Output			*/
#define TCAYT		(u_char)246	/* "Are You There?" Function	*/
#define TCEC		(u_char)247	/* Erase Character		*/
#define TCEL		(u_char)248	/* Erase Line			*/
#define TCGA		(u_char)249	/* "Go Ahead" Function		*/
#define TCWILL		(u_char)251	/* Desire/Confirm Will Do Option*/
#define TCWONT		(u_char)252	/* Refusal To Do Option		*/
#define TCDO		(u_char)253	/* Request To Do Option		*/
#define TCDONT		(u_char)254	/* Request NOT To Do Option	*/
#define TCIAC		(u_char)255	/* Interpret As Command Escape	*/

/* Telnet Option Codes: */
#define TOTXBINARY	(u_char)  0	/* TRANSMIT-BINARY option	*/
#define TOECHO		(u_char)  1	/* ECHO Option			*/
#define TONOGA		(u_char)  3	/* Suppress Go-Ahead Option	*/
#define TOTERMTYPE	(u_char) 24	/* Terminal-Type Option		*/
#define TONAWS		(u_char) 31	/* Window  Size */

/* Network Virtual Printer Special Characters: */
/* In normal situations, these characters will to translated into local
   control characters , then pass to upper layer term. But in our situation,
   we can pass them to term directly */

#define VPLF		'\n'	/* Line Feed				*/
#define VPCR		'\r'	/* Carriage Return			*/
#define VPBEL		'\a'	/* Bell (attention signal)		*/
#define VPBS		'\b'	/* Back Space				*/
#define VPHT		'\t'	/* Horizontal Tab			*/
#define VPVT		'\v'	/* Vertical Tab				*/
#define VPFF		'\f'	/* Form Feed				*/

/* Keyboard Command Characters: */

/* Option Subnegotiation Constants: */
#define TT_IS		0	/* TERMINAL_TYPE option "IS" command	*/
#define TT_SEND		1	/* TERMINAL_TYPE option "SEND" command	*/

/* Telnet Socket-Input FSM States: */
#define TSDATA		 0	/* normal data processing		*/
#define TSIAC		 1	/* have seen IAC			*/
#define TSWOPT		 2	/* have seen IAC-{WILL/WONT}		*/
#define TSDOPT		 3	/* have seen IAC-{DO/DONT}		*/
#define TSSUBNEG	 4	/* have seen IAC-SB			*/
#define TSSUBIAC	 5	/* have seen IAC-SB-...-IAC		*/
// if any state added here, please update NTSTATES (currently 6).

// Telnet Option Subnegotiation FSM States:
#define SS_START	0		// initial state
#define SS_TERMTYPE	1		// TERMINAL_TYPE option subnegotiation
#define SS_END		2		// state after all legal input
// if any state added here, please update NSSTATES (currently 3).

#define FSINVALID	0xff		// an invalid state number
#define TCANY		(NCHRS+1)	// match any character

#define TINVALID	0xff		// an invalid transition index

namespace FQTerm {

struct fsm_trans FQTermTelnet::ttstab[] =  {
  /* State	Input		Next State	Action	*/
  /* ------	------		-----------	-------	*/
  {
    TSDATA, TCIAC, TSIAC, &FQTermTelnet::no_op
  }, {
    TSDATA, TCANY, TSDATA, &FQTermTelnet::ttputc
  }, {
    TSIAC, TCIAC, TSDATA, &FQTermTelnet::ttputc
  }, {
    TSIAC, TCSB, TSSUBNEG, &FQTermTelnet::no_op
  }, {/* Telnet Commands */
    TSIAC, TCNOP, TSDATA, &FQTermTelnet::no_op
  }, {
    TSIAC, TCDM, TSDATA, &FQTermTelnet::tcdm
  }, {  /* Option Negotiation */
    TSIAC, TCWILL, TSWOPT, &FQTermTelnet::recopt
  }, {
    TSIAC, TCWONT, TSWOPT, &FQTermTelnet::recopt
  }, {
    TSIAC, TCDO, TSDOPT, &FQTermTelnet::recopt
  }, {
    TSIAC, TCDONT, TSDOPT, &FQTermTelnet::recopt
  }, {
    TSIAC, TCANY, TSDATA, &FQTermTelnet::no_op
  }, { /* Option Subnegotion */
    TSSUBNEG, TCIAC, TSSUBIAC, &FQTermTelnet::no_op
  }, {
    TSSUBNEG, TCANY, TSSUBNEG, &FQTermTelnet::subopt
  }, {
    TSSUBIAC, TCSE, TSDATA, &FQTermTelnet::subend
  }, {
    TSSUBIAC, TCANY, TSSUBNEG, &FQTermTelnet::subopt
  }, {
    TSWOPT, TOECHO, TSDATA, &FQTermTelnet::do_echo
  }, {
    TSWOPT, TONOGA, TSDATA, &FQTermTelnet::do_noga
  }, {
    TSWOPT, TOTXBINARY, TSDATA, &FQTermTelnet::do_txbinary
  }, {
    TSWOPT, TCANY, TSDATA, &FQTermTelnet::do_notsup
  },{
    TSDOPT, TONAWS, TSDATA, &FQTermTelnet::will_naws
  }, {
    TSDOPT, TOTERMTYPE, TSDATA, &FQTermTelnet::will_termtype
  }, {
    TSDOPT, TOTXBINARY, TSDATA, &FQTermTelnet::will_txbinary
  }, {
    TSDOPT, TCANY, TSDATA, &FQTermTelnet::will_notsup
  }, {
    FSINVALID, TCANY, FSINVALID, &FQTermTelnet::tnabort
  },
};


struct fsm_trans FQTermTelnet::substab[] =  {
  /* State        Input           Next State      Action  */
  /* ---------------------------------------------------- */
  {
    SS_START, TOTERMTYPE, SS_TERMTYPE, &FQTermTelnet::no_op
  }, {
    SS_START, TCANY, SS_END, &FQTermTelnet::no_op
  }, {
    SS_TERMTYPE, TT_SEND, SS_END, &FQTermTelnet::subtermtype
  }, {
    SS_TERMTYPE, TCANY, SS_END, &FQTermTelnet::no_op
  }, {
    SS_END, TCANY, SS_END, &FQTermTelnet::no_op
  }, {
    FSINVALID, TCANY, FSINVALID, &FQTermTelnet::tnabort
  },
};



/*------------------------------------------------------------------------
 * Constructor
 *------------------------------------------------------------------------
 */
FQTermTelnet::FQTermTelnet(const QString &strTermType, int rows, int columns,
                           int protocolType, int hostType, const char *sshuser, const char *sshpasswd)
    : from_socket(),
      to_ansi(),
      from_ansi(),
      to_socket(),
      hostType_(hostType),
      protocolType_(protocolType) {
  term = new char[21];
  memset(term, 0, 21*sizeof(char));
  // TODO: clean up, need???
#ifdef WIN32
  _snprintf(term, sizeof(term), "%s", strTermType.toLatin1().constData());
#else
  strncpy(term,strTermType.toLatin1(),20);
#endif

  wx = columns;
  wy = rows;
  wsize = 0;
  done_naws = 0;
  synching = 0;
  doecho = 0;
  sndbinary = 0;
  rcvbinary = 0;
  noga = 0;
  termtype = 0;
  naws = 0;
  server_sent_do_naws = 0;
  raw_size = 0;

#ifndef _NO_SSH_COMPILED
  if (protocolType == 1) {
    socket = new FQTermSSHSocket(columns, rows, strTermType, sshuser, sshpasswd);
    FQ_VERIFY(connect(socket, SIGNAL(sshAuthOK()),
		      this, SIGNAL(onSSHAuthOK())));
  } else if (protocolType == 2) {
    socket = new FQTermLocalSocket();
  } else {
    socket = new FQTermTelnetSocket();
  }
#else
  socket = new FQTermTelnetSocket();
#endif

  // connect signal and slots
  FQ_VERIFY(connect(socket, SIGNAL(connected()), this, SLOT(connected())));
  FQ_VERIFY(connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead())));
  FQ_VERIFY(connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(showError(QAbstractSocket::SocketError))));
  FQ_VERIFY(connect(socket, SIGNAL(hostFound()), this, SLOT(hostFound())));
  FQ_VERIFY(connect(socket, SIGNAL(delayedCloseFinished()),
                    this, SLOT(delayCloseFinished())));
  FQ_VERIFY(connect(socket, SIGNAL(connectionClosed()), this, SLOT(closed())));
  FQ_VERIFY(connect(socket, SIGNAL(socketState(int)), this, SIGNAL(TelnetState(int))));

  FQ_VERIFY(connect(socket, SIGNAL(requestUserPwd(QString *, QString *, bool *)), 
                    this, SIGNAL(requestUserPwd(QString *, QString *, bool *))));

  FQ_VERIFY(connect(socket, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString))));

  // Init telnet, mainly the FSMs
  init_telnet();
}

/*------------------------------------------------------------------------
 * destructor
 *------------------------------------------------------------------------
 */
FQTermTelnet::~FQTermTelnet() {
  // delete objects
  delete [] term;
  delete socket;
}



/*------------------------------------------------------------------------
 * init_telnet
 *------------------------------------------------------------------------
 */
void FQTermTelnet::init_telnet() {
  fsmbuild(); /* setup FSMs */
}


/*------------------------------------------------------------------------
 * fsmbuild - build the Finite State Machine data structures
 *------------------------------------------------------------------------
 */
void FQTermTelnet::fsmbuild() {
  fsminit(ttfsm, ttstab, NTSTATES);
  ttstate = TSDATA;

  fsminit(subfsm, substab, NSSTATES);
  substate = SS_START;

}

/*------------------------------------------------------------------------
 * fsminit - Finite State Machine initializer
 *------------------------------------------------------------------------
 */
void FQTermTelnet::fsminit(u_char fsm[][NCHRS], struct fsm_trans ttab[], int
                           nstates) {
  struct fsm_trans *pt;
  int sn, ti, cn;

  for (cn = 0; cn < NCHRS; ++cn) {
    for (ti = 0; ti < nstates; ++ti) {
      fsm[ti][cn] = TINVALID;
    }
  }

  for (ti = 0; ttab[ti].ft_state != FSINVALID; ++ti) {
    pt = &ttab[ti];
    sn = pt->ft_state;
    if (pt->ft_char == TCANY) {
      for (cn = 0; cn < NCHRS; ++cn) {
        if (fsm[sn][cn] == TINVALID) {
          fsm[sn][cn] = ti;
        }
      }
    } else {
      fsm[sn][pt->ft_char] = ti;
    }
  }

  /* set all uninitialized indices to an invalid transition */
  for (cn = 0; cn < NCHRS; ++cn) {
    for (ti = 0; ti < nstates; ++ti) {
      if (fsm[ti][cn] == TINVALID) {
        fsm[ti][cn] = ti;
      }
    }
  }

}

/*------------------------------------------------------------------------
 * connect to host
 *------------------------------------------------------------------------
 */
void FQTermTelnet::connectHost(const QString &hostname, quint16 portnumber) {
  done_naws = 0;
  synching = 0;
  doecho = 0;
  sndbinary = 0;
  rcvbinary = 0;
  noga = 0;
  termtype = 0;
  naws = 0;

  socket->connectToHost(hostname, portnumber);
  // host name resolving
  emit TelnetState(TSRESOLVING);
}

void FQTermTelnet::windowSizeChanged(int x, int y) {
  wx = x;
  wy = y;
  if (bConnected) {
    if (hostType_ == 1 && (protocolType_ == 1 || protocolType_ == 2)) {
      //This is a *nix host, with ssh connection.
      socket->setTermSize(x, y);
      return;
    }
    naws = 0;

    /*
    QByteArray cmd(10, 0);
    cmd[0] = (char)TCIAC;
    cmd[1] = (char)TCSB;
    cmd[2] = (char)TONAWS;
    cmd[3] = (char)(short(wx) >> 8);
    cmd[4] = (char)(short(wx) & 0xff);
    cmd[5] = (char)(short(wy) >> 8);
    cmd[6] = (char)(short(wy) & 0xff);
    cmd[7] = (char)TCIAC;
    cmd[8] = (char)TCSE;
    socket->writeBlock(cmd);
    */

  }
}

/*------------------------------------------------------------------------
 * set proxy
 *-----------------------------------------------------------------------
 */
void FQTermTelnet::setProxy(int nProxyType, bool bAuth, const QString
                            &strProxyHost, quint16 uProxyPort,
                            const QString &strProxyUsr,
                            const QString &strProxyPwd) {
  socket->setProxy(nProxyType, bAuth, strProxyHost,
                   uProxyPort, strProxyUsr, strProxyPwd);
}

/*------------------------------------------------------------------------
 * close connection
 *-----------------------------------------------------------------------
 */

void FQTermTelnet::close() {
  socket->close();
}

/*------------------------------------------------------------------------
 * SLOT connected
 *------------------------------------------------------------------------
 */
void FQTermTelnet::connected() {
  bConnected = true;
  emit TelnetState(TSHOSTCONNECTED);
}

/*------------------------------------------------------------------------
 * SLOT closed
 *------------------------------------------------------------------------
 */

void FQTermTelnet::closed() {
  bConnected = false;
  emit TelnetState(TSCLOSED);
}

/*------------------------------------------------------------------------
 * SLOT delayCloseFinished
 *------------------------------------------------------------------------
 */

void FQTermTelnet::delayCloseFinished() {
  bConnected = false;
  emit TelnetState(TSCLOSEFINISH);
}

/*------------------------------------------------------------------------
 * SLOT hostFound
 *------------------------------------------------------------------------
 */
void FQTermTelnet::hostFound() {
  emit TelnetState(TSHOSTFOUND);
}

/*------------------------------------------------------------------------
 * SLOT error
 *------------------------------------------------------------------------
 */
void FQTermTelnet::showError(QAbstractSocket::SocketError index) {

  switch (index) {
    case QAbstractSocket::ConnectionRefusedError: emit TelnetState(TSREFUSED);
      break;
    case QAbstractSocket::HostNotFoundError: emit TelnetState(TSHOSTNOTFOUND);
      break;
      //FIXME: am I right
    case QAbstractSocket::SocketAccessError: emit TelnetState(TSREADERROR);
      break;
    case QAbstractSocket::RemoteHostClosedError: emit TelnetState(TSCLOSED);
      break;
    default:
      emit TelnetState(TSERROR);
      ;
      break;
  }

}

/*------------------------------------------------------------------------
 * SLOOT socketReadyRead -
 * 	when socket has data to upload, it send out readyRead() signal, it
 * 	invokes this SLOT to read data in, do telnet decode job, and send out
 * 	readyRead() SIGNAL to upper layer
 *------------------------------------------------------------------------
 */
void FQTermTelnet::socketReadyRead() {
  int nbytes, nread;
  // TODO_UTF16: check this function.
  
  // get the data size
  nbytes = socket->bytesAvailable();
  if (nbytes <= 0) {
    return ;
  }

  raw_size = nbytes;

  //resize input buffer
  from_socket.resize(0);

  //read data from socket to from_socket
  from_socket = socket->readBlock(nbytes);
  nread = from_socket.size();
  //do some checks
  if (nread <= 0) {
    FQ_TRACE("telnet", 8) << "Failed to read socket: " << nread << " <= 0.";
    return ;
  }
  if (nread > nbytes) {
    FQ_TRACE("telnet", 0) << "Overflow when reading socket: "
                          << " nread = " << nread
                          << " > nbytes = " << nbytes;
    return ;
  }

  //resize output buffer
  to_ansi.resize(2 *nread);
  to_socket.resize(4 *nread);

  rsize = 0;
  wsize = 0;

  // do telnet decode job...
  struct fsm_trans *pt;
  int i, ti;

  u_char c;
  for (i = 0; i < nread; ++i) {
    c = (u_char)(from_socket[i]);
    ti = ttfsm[ttstate][c];
    pt = &ttstab[ti];
    (this->*(pt->ft_action))((int)c);
    ttstate = pt->ft_next;
  }

  // flush the to_socket buffer, it contain response to server
  if (wsize > 0) {
    socket->writeBlock(to_socket.left(wsize));
    socket->flush();
  }

  /* send SIGNAL readyRead() with the size of data available*/
  if (rsize > 0 || raw_size > 0) {
    emit readyRead(rsize, raw_size);
  }

}

int FQTermTelnet::raw_len() {
  return raw_size;
}

/*------------------------------------------------------------------------
 * actions
 *------------------------------------------------------------------------
 */
int FQTermTelnet::read_raw(char *data, uint maxlen) {
  //do some checks
  if (data == 0) {
    FQ_TRACE("telnet", 0) << "NULL pointer.";
    return -1;
  }
  if ((long)maxlen < raw_size) {
    /* we need all data be read out in one read */
    FQ_TRACE("telnet", 0) << "The upper layer's buffer is too small.";
    return -1;
  }

  //do it, memcpy( destination, source, size)
  memcpy(data, from_socket.data(), raw_size);
  return raw_size;
}


/*------------------------------------------------------------------------
 * actions
 *------------------------------------------------------------------------
 */
int FQTermTelnet::read(char *data, uint maxlen) {
  //do some checks
  if (data == 0) {
    FQ_TRACE("telent", 0) << "NULL pointer.";
    return -1;
  }
  if (maxlen < rsize) {
    /* we need all data be read out in one read */
    FQ_TRACE("telnet", 0) << "The upper layer's buffer is too small.";
    return -1;
  }

  //do it, memcpy( destination, source, size)
  memcpy(data, to_ansi.data(), rsize);

  FQ_TRACE("telnet", 8) << "read " << rsize << " bytes:\n"
                        << dumpHexString << std::string(data, rsize);

  return rsize;
}

/*------------------------------------------------------------------------
 * actions
 *------------------------------------------------------------------------
 */


/*------------------------------------------------------------------------
 * writeBlock
 * 	write data from data-> to socket, the length of data is len
 *------------------------------------------------------------------------
 */
int FQTermTelnet::write(const char *data, uint len) {
  // TODO: accept data, (This seems can be removed????)
  from_ansi.resize(len);
  memcpy(from_ansi.data(), data, len);

  // resize output buffer
  to_socket.resize(2 *len);
  wsize = 0;

  // process keyboard input
  // because we use GUI, there is no need to support a "command mode"
  // So the keyboard-input FSM isnt' necessary.

  uint i;

  u_char c; // TODO: for gcc's happy :)
  for (i = 0; i < len; ++i) {
    c = (u_char)(from_ansi[i]);
    soputc((int)c);
  }

  FQ_TRACE("telnet", 2) << "write " << len << " bytes:\n" 
                        << dumpHexString << std::string(data, len);

  //flush socket
  socket->writeBlock(to_socket.left(wsize));
  socket->flush();

  emit TelnetState(TSWRITED);
  return 0;
}


/*------------------------------------------------------------------------
 * Trans functions
 * All trans functions use from_socket, to_ansi, to_socket buffers, and
 * rsize, wsize .
 *------------------------------------------------------------------------
 */

/*------------------------------------------------------------------------
 * tcdm - handle the telnet "DATA MARK" command (marks end of SYNCH)
 *------------------------------------------------------------------------
 */
int FQTermTelnet::tcdm(int) {
  if (synching > 0) {
    synching--;
  }
  return 0;
}

/*------------------------------------------------------------------------
 * rcvurg - receive urgent data input (indicates a telnet SYNCH)
 *------------------------------------------------------------------------
 */
/*
  int FQTermTelnet::rcvurg(int sig)
  {
  synching++;
  }
*/

/*------------------------------------------------------------------------
 * recopt - record option type
 *------------------------------------------------------------------------
 */

int FQTermTelnet::recopt(int c) {
  option_cmd = c;
  return 0;
}

/*------------------------------------------------------------------------
 * no_op - do nothing
 *------------------------------------------------------------------------
 */

int FQTermTelnet::no_op(int) {
  return 0;
}


/*------------------------------------------------------------------------
 * do_echo - handle TELNET WILL/WON'T ECHO option
 *------------------------------------------------------------------------
 */
int FQTermTelnet::do_echo(int c) {
  if (doecho) {
    if (option_cmd == TCWILL) {
      return 0;
    }
    /* already doing ECHO		*/
  } else if (option_cmd == TCWONT) {
    return 0;
  }
  /* already NOT doing ECHO	*/

  doecho = !doecho;

  putc_down(TCIAC);
  if (doecho) {
    putc_down(TCDO);
  } else {
    putc_down(TCDONT);
  }
  putc_down((char)c);
  return 0;
}

/*------------------------------------------------------------------------
 * putc_down - put a character in to_socket buffer.
 * wsize represent the number of bytes in to_socket buffer, and the buffer
 * is addressed from 0, NOT 1.
 *------------------------------------------------------------------------
 */
void FQTermTelnet::putc_down(u_char c) {
  // check overflow
  if ((long)(wsize + 1) > to_socket.size()) {
    FQ_TRACE("telnet", 0) << "Buffer to_socket overflow.";
    return ;
  }
  // put it in the buffer
  //to_socket->replace(wsize, 1, (const char *)&c);
  to_socket[wsize] = c;

  wsize++;
  return ;
}

/*------------------------------------------------------------------------
 * do_notsup - handle an unsupported telnet "will/won't" option
 *------------------------------------------------------------------------
 */

int FQTermTelnet::do_notsup(int c) {
  putc_down(TCIAC);
  putc_down(TCDONT);
  putc_down((char)c);
  return 0;
}

/*------------------------------------------------------------------------
 * do_noga - don't do telnet Go-Ahead's
 *------------------------------------------------------------------------
 */

int FQTermTelnet::do_noga(int c) {
  if (noga) {
    if (option_cmd == TCWILL) {
      return 0;
    }
  } else if (option_cmd == TCWONT) {
    return 0;
  }

  noga = !noga;

  putc_down(TCIAC);
  if (noga) {
    putc_down(TCDO);
  } else {
    putc_down(TCDONT);
  }
  putc_down((char)c);
  return 0;
}

/*------------------------------------------------------------------------
 * do_txbinary - handle telnet "will/won't" TRANSMIT-BINARY option
 *------------------------------------------------------------------------
 */

int FQTermTelnet::do_txbinary(int c) {
  if (rcvbinary) {
    if (option_cmd == TCWILL) {
      return 0;
    }
  } else if (option_cmd == TCWONT) {
    return 0;
  }

  rcvbinary = !rcvbinary;

  putc_down(TCIAC);
  if (rcvbinary) {
    putc_down(TCDO);
  } else {
    putc_down(TCDONT);
  }
  putc_down((char)c);
  return 0;
}

/*------------------------------------------------------------------------
 * will_notsup - handle an unsupported telnet "do/don't" option
 *------------------------------------------------------------------------
 */

int FQTermTelnet::will_notsup(int c) {
  putc_down(TCIAC);
  putc_down(TCWONT);
  putc_down((char)c);
  return 0;
}

/*------------------------------------------------------------------------
 * will_txbinary - handle telnet "do/don't" TRANSMIT-BINARY option
 *------------------------------------------------------------------------
 */

int FQTermTelnet::will_txbinary(int c) {
  if (sndbinary) {
    if (option_cmd == TCDO) {
      return 0;
    }
  } else if (option_cmd == TCDONT) {
    return 0;
  }

  sndbinary = !sndbinary;

  putc_down(TCIAC);
  if (sndbinary) {
    putc_down(TCWILL);
  } else {
    putc_down(TCWONT);
  }
  putc_down((char)c);
  return 0;
}

/*------------------------------------------------------------------------
 * will_termtype - handle telnet "do/don't" TERMINAL-TYPE option
 *------------------------------------------------------------------------
 */
int FQTermTelnet::will_termtype(int c) {
  if (termtype) {
    if (option_cmd == TCDO) {
      return 0;
    }
  } else if (option_cmd == TCDONT) {
    return 0;
  }

  termtype = !termtype;

  putc_down(TCIAC);

  if (termtype) {
    putc_down(TCWILL);
  } else {
    putc_down(TCWONT);
  }

  putc_down((char)c);

  // TODO: Do NOT use this assume! some foolish BBS not response the request
  /* The client expects that once the remote application receives
     terminal type information it will send control sequences for
     the terminal, which cannot be sent using the NVT encoding, So
     change the transfer mode to binary in both directions */
  /* set up binary data path; send WILL, DO */
  /*	if (termtype) {
        option_cmd = TCWILL;
        do_txbinary(TOTXBINARY);
        option_cmd = TCDO;
        will_txbinary(TOTXBINARY);
        }
  */
  return 0;
}

int FQTermTelnet::will_naws(int c) {
  if (naws) {
    if (option_cmd == TCDO)
      return 0;
  } else if (option_cmd == TCDONT)
    return 0;


  naws = !naws;

  putc_down(TCIAC);
  if (naws)
    putc_down(TCWILL);
  else
    putc_down(TCWONT);
  putc_down((char)c);

  putc_down(TCIAC);
  putc_down(TCSB);
  putc_down(TONAWS);
  putc_down((char)(short(wx) >> 8));
  putc_down((char)(short(wx)&0xff));
  putc_down((char)(short(wy) >> 8));
  putc_down((char)(short(wy)&0xff));
  putc_down(((char)TCIAC));
  putc_down((char)TCSE);

  return 0;
}

/*------------------------------------------------------------------------
 * subopt - do option subnegotiation FSM transitions
 *------------------------------------------------------------------------
 */
int FQTermTelnet::subopt(int c) {
  struct fsm_trans *pt;
  int ti;

  ti = subfsm[substate][c];
  pt = &substab[ti];
  (this->*(pt->ft_action))(c);
  substate = pt->ft_next;
  return 0;
}

/*------------------------------------------------------------------------
 * subtermtype - do terminal type option subnegotation
 *------------------------------------------------------------------------
 */

int FQTermTelnet::subtermtype(int) {
  char *i;
  /* have received IAC.SB.TERMTYPE.SEND */

  putc_down(TCIAC);
  putc_down(TCSB);
  putc_down(TOTERMTYPE);
  putc_down(TT_IS);

  //write term type string
  //fputs(term, sfp);
  for (i = term; (*i) != '\000'; i++) {
    putc_down(*i);
  }

  putc_down(TCIAC);
  putc_down(TCSE);
  return 0;
}

/*------------------------------------------------------------------------
 * subend - end of an option subnegotiation; reset FSM
 *------------------------------------------------------------------------
 */

int FQTermTelnet::subend(int) {
  substate = SS_START;
  return 0;
}

/*------------------------------------------------------------------------
 * soputc - move a character from the keyboard to the socket
 * 	    convert an character into the NVT encoding and send it
 *	    through the socket to the server.
 *------------------------------------------------------------------------
 */

int FQTermTelnet::soputc(int c) {
  if (sndbinary) {
    if (c == TCIAC) {
      putc_down(TCIAC);
    }
    /* byte-stuff IAC	*/
    putc_down(c);
    return 0;
  }

  //c &= 0x7f;	/* 7-bit ASCII only ???*/
  // Convert local special characters to NVT characters
  /* TODO: // BBS don't need control signals
     if (c == t_intrc || c == t_quitc) {	// Interrupt
     putc_down(TCIAC);
     putc_down(TCIP);
     } else if (c == sg_erase) {		// Erase Char
     putc_down(TCIAC);
     putc_down(TCEC);
     } else if (c == sg_kill) {		// Erase Line
     putc_down(TCIAC);
     putc_down(TCEL);
     } else if (c == t_flushc) {		// Abort Output
     putc_down(TCIAC);
     putc_down(TCAO);
     } else  */
  putc_down((char)c);

  return 0;
}

/*------------------------------------------------------------------------
 * xputc - putc to upper layer with optional file scripting
 *------------------------------------------------------------------------
 */
int FQTermTelnet::xputc_up(char ch) {
  if ((long)(rsize + 1) > to_ansi.size()) {
    FQ_TRACE("telnet", 0) << "Buffer to_ansi overflow.";
    return - 1;
  }
  //to_ansi->replace(wsize, 1, &ch);
  to_ansi[rsize] = u_char(ch);
  rsize++;

  return 0;
}

/*------------------------------------------------------------------------
 * xfputs - fputs with optional file scripting
 *------------------------------------------------------------------------
 */
int FQTermTelnet::xputs_up(char *str) {
  /*if (scrfp)
	fputs(str, scrfp);*/

  char *i;
  for (i = str; (*i) != '\000'; i++) {
    xputc_up(*i);
  }
  return 0;
}

/*------------------------------------------------------------------------
 * ttputc - print a single character on a Network Virtual Terminal
 *------------------------------------------------------------------------
 */
int FQTermTelnet::ttputc(int c) {
  if (rcvbinary) {
    xputc_up((char)c); /* print uninterpretted	*/
    return 0;
  }
  /* no data, if in SYNCH	*/
  /*
	if (synching)
	return 0;
  */
  /* TODO: FQTermTelnet doesnot interpret NVT code, provide datas to upper
     layer directly. So, <cr><lf>	will not be replaced with <lf>
  */
  xputc_up((char)c);

  return 0;
}

/*------------------------------------------------------------------------
 * invalid state reached, aborted
 *------------------------------------------------------------------------
 */
int FQTermTelnet::tnabort(int) {
  FQ_VERIFY(false); // "invalid state reached, aborted";
  //	exit(-1);
  return -1;
}

bool FQTermTelnet::readyForInput()
{
  return socket->readyForInput();
}



}  // namespace FQTerm

#include "fqterm_telnet.moc"
