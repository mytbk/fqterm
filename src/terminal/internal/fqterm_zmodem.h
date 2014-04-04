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

#ifndef FQTERM_ZMODEM_H
#define FQTERM_ZMODEM_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <QByteArray>
#include <QObject>
#include <QTimer>
#include <QFile>
#include <QStringList>

namespace FQTerm {

/* PARAMETERS
 *
 * The following #defines control the behavior of the Zmodem
 * package.  Note that these may be replaced with variables
 * if you like.  For example, "#define DoInitRZ" may be replaced
 * with "extern int DoInitRz" to use a global variable, or with
 * "#define DoInitRZ (info->doInitRz)" to use a variable you
 * add to the ZModem structure.
 *
 * It is assumed that the compiler is good enough to optimize
 * "if( 0 )" and "if( 1 )" cases.  If not, you may wish to modify
 * the source code to use #ifdef instead.
 */

#define DoInitRZ	1	/* send initial "rz\r" when transmitting */
#define AllowCommand	0	/* allow remote end to execute commands */
#define SendSample	1	/* sender can sample reverse channel */
#define SendAttn	1	/* sender can be interrupted with Attn signal */
#define ResponseTime	10	/* reasonable response time for sender to
                             * respond to requests from receiver */
#define SerialNo	1	/* receiver serial # */
#define MaxNoise	64	/* max "noise" characters before transmission
                         * pauses */
#define MaxErrs		30	/* Max receive errors before cancel */
#define AlwaysSinit	1	/* always send ZSINIT header, even if not
                         * needed, this makes protocol more robust */

#define SendOnly	0	/* compiles smaller version for send only */
#define RcvOnly		0	/* compiles smaller version for receive only */

enum enum_InputState {
  Idle, Padding, Inhdr, Indata, Finish, Ysend, Yrcv
};
enum enum_Protocol {
  XMODEM, YMODEM, ZMODEM
};
enum enum_Streaming {
  Full, StrWindow, SlidingWindow, Segmented
};
enum enum_transferstate {
  notransfer, transferstart, transferstop
};


/* Internal State */

typedef enum zmstate {
  /* receive */
  RStart,  /* sent RINIT, waiting for ZFILE or SINIT */
  RSinitWait,  /* got SINIT, waiting for data */
  RFileName,  /* got ZFILE, waiting for filename & info */
  RCrc,  /* got filename, want crc too */
  RFile,  /* got filename, ready to read */
  RData,  /* reading data */
  RDataErr,  /* encountered error, ignoring input */
  RFinish,  /* sent ZFIN, waiting for 'OO' */

  /* transmit */
  TStart,  /* waiting for INIT frame from other end */
  TInit,  /* received INIT, sent INIT, waiting for ZACK */
  FileWait,  /* sent file header, waiting for ZRPOS */
  CrcWait,  /* sent file crc, waiting for ZRPOS */
  Sending,  /* sending data subpackets, ready for int */
  SendWait,  /* waiting for ZACK */
  SendDone,  /* file finished, need to send EOF */
  SendEof,  /* sent EOF, waiting for ZACK */
  TFinish,  /* sent ZFIN, waiting for ZFIN */

  /* general */
  CommandData,  /* waiting for command data */
  CommandWait,  /* waiting for command to execute */
  StderrData,  /* waiting for stderr data */
  Done,

  /* x/ymodem transmit */
  YTStart,  /* waiting for 'G', 'C' or NAK */
  YTFile,  /* sent filename, waiting for ACK */
  YTDataWait,  /* ready to send data, waiting for 'C' */
  YTData,  /* sent data, waiting for ACK */
  YTEOF,  /* sent eof, waiting for ACK */
  YTFin,  /* sent null filename, waiting for ACK */

  /* x/ymodem receive */
  YRStart,  /* sent 'C', waiting for filename */
  YRDataWait,  /* received filename, waiting for data */
  YRData,  /* receiving filename or data */
  YREOF /* received first EOT, waiting for 2nd */

} ZMState;




typedef struct {
  int ifd; /* input fd, for use by caller's routines */
  int ofd; /* output fd, for use by caller's routines */
  FILE *file; /* file being transfered */
  int zrinitflags; /* receiver capabilities, see below */
  int zsinitflags; /* sender capabilities, see below */
  char *attn; /* attention string, see below */
  int timeout; /* timeout value, in seconds */
  int bufsize; /* receive buffer size, bytes */
  int packetsize; /* preferred transmit packet size */
  int windowsize; /* max window size */

  /* file attributes: read-only */

  int filesRem, bytesRem;
  uchar f0, f1, f2, f3; /* file flags */
  int len, mode, fileType; /* file flags */
  ulong date; /* file date */

  /* From here down, internal to Zmodem package */

  ZMState state; /* protocol internal state */
  char *filename; /* filename */
  char *rfilename; /* remote filename */
  int crc32; /* use 32-bit crc */
  int pktLen; /* length of this packet */
  int DataType; /* input data type */
  int PacketType; /* type of this packet */
  int rcvlen;
  int chrCount; /* chars received in current header/buffer */
  int crcCount; /* crc characters remaining at end of buffer */
  int canCount; /* how many CAN chars received? */
  int noiseCount; /* how many noise chars received? */
  int errorFlush; /* ignore incoming data because of error */
  uchar *buffer; /* data buffer */
  ulong offset; /* file offset */
  ulong lastOffset; /* last acknowledged offset */
  ulong zrposOffset; /* last offset specified w/zrpos */
  int ylen, bufp; /* len,location of last Ymodem packet */
  int fileEof; /* file eof reached */
  int packetCount; /* # packets received */
  int errCount; /* how many data errors? */
  int timeoutCount; /* how many times timed out? */
  int windowCount; /* how much data sent in current window */
  int atSign; /* last char was '@' */
  int lastCR; /* last char was CR */
  int escCtrl; /* other end requests ctrl chars be escaped */
  int escHibit; /* other end requests hi bit be escaped */
  int escape; /* next character is escaped */
  int interrupt; /* received attention signal */
  int waitflag; /* next send should wait */
  /* parser state */
  //	  enum enum_InputState {Idle, Padding, Inhdr, Indata, Finish, Ysend, Yrcv} InputState ;
  enum_InputState InputState;
  //	  enum enum_Protocol {XMODEM, YMODEM, ZMODEM} Protocol ;
  enum_Protocol Protocol;
  uchar hdrData[9]; /* header type and data */
  uchar fileFlags[4]; /* file xfer flags */
  ulong crc; /* crc of incoming header/data */
  //	  enum enum_Streaming {Full, StrWindow, SlidingWindow, Segmented} Streaming ;
  enum_Streaming Streaming;
} ZModem;



/* ZRINIT flags.  Describe receiver capabilities */

#define CANFDX	1	/* Rx is Full duplex */
#define CANOVIO	2	/* Rx can overlap I/O */
#define CANBRK	4	/* Rx can send a break */
#define CANCRY	010	/* Rx can decrypt */
#define CANLZW	020	/* Rx can uncompress */
#define CANFC32	040	/* Rx can use 32-bit crc */
#define ESCCTL	0100	/* Rx needs control chars escaped */
#define ESC8	0200	/* Rx needs 8th bit escaped. */

/* ZSINIT flags.  Describe sender capabilities */

#define TESCCTL	0100	/* Tx needs control chars escaped */
#define TESC8	0200	/* Tx needs 8th bit escaped. */


/* ZFILE transfer flags */

/* F0 */
#define ZCBIN	1	/* binary transfer */
#define ZCNL	2	/* convert NL to local eol convention */
#define ZCRESUM	3	/* resume interrupted file xfer, or append to a
                       growing file. */

/* F1 */
#define ZMNEWL	1	/* transfer if source newer or longer */
#define ZMCRC	2	/* transfer if different CRC or length */
#define ZMAPND	3	/* append to existing file, if any */
#define ZMCLOB	4	/* replace existing file */
#define ZMNEW	5	/* transfer if source is newer */
#define ZMDIFF	6	/* transfer if dates or lengths different */
#define ZMPROT	7	/* protect: transfer only if dest doesn't exist */
#define ZMCHNG	8	/* change filename if destination exists */
#define ZMMASK	037	/* mask for above. */
#define ZMSKNOLOC 0200	/* skip if not present at Rx end */

/* F2 */
#define ZTLZW	1	/* lzw compression */
#define ZTRLE	3	/* run-length encoding */

/* F3 */
#define ZCANVHDR 1	/* variable headers ok */
#define ZRWOVR	4	/* byte position for receive window override/256 */
#define ZXSPARS	64	/* encoding for sparse file ops. */



/* ATTN string special characters.  All other characters sent verbose */

#define ATTNBRK	'\335'	/* send break signal */
#define ATTNPSE	'\336'	/* pause for one second */

/* error code definitions [O] means link still open */

#define ZmDone		-1	/* done */
#define ZmErrInt	-2	/* internal error */
#define ZmErrSys	-3	/* system error, see errno */
#define ZmErrNotOpen	-4	/* communication channel not open */
#define ZmErrCantOpen	-5	/* can't open file, see errno [O] */
#define ZmFileTooLong	-6	/* remote filename too long [O] */
#define ZmFileCantWrite	-7	/* could not write file, see errno */
#define ZmDataErr	-8	/* too many data errors */
#define ZmErrInitTo	-10	/* transmitter failed to respond to init req. */
#define ZmErrSequence	-11	/* packet received out of sequence */
#define ZmErrCancel	-12	/* cancelled by remote end */
#define ZmErrRcvTo	-13	/* remote receiver timed out during transfer */
#define ZmErrSndTo	-14	/* remote sender timed out during transfer */
#define ZmErrCmdTo	-15	/* remote command timed out */



/* ZModem character definitions */

#define ZDLE	030	/* zmodem escape is CAN */
#define ZPAD	'*'	/* pad */
#define ZBIN	'A'	/* introduces 16-bit crc binary header */
#define ZHEX	'B'	/* introduces 16-bit crc hex header */
#define ZBIN32	'C'	/* introduces 32-bit crc binary header */
#define ZBINR32	'D'	/* introduces RLE packed binary frame w/32-bit crc */
#define ZVBIN	'a'	/* alternate ZBIN */
#define ZVHEX	'b'	/* alternate ZHEX */
#define ZVBIN32	'c'	/* alternate ZBIN32 */
#define ZVBINR32 'd'	/* alternate ZBINR32 */
#define ZRESC	0177	/* RLE flag/escape character */



/* ZModem header type codes */

#define ZRQINIT	0	/* request receive init */
#define ZRINIT	1	/* receive init */
#define ZSINIT	2	/* send init sequence, define Attn */
#define ZACK	3	/* ACK */
#define ZFILE	4	/* file name, from sender */
#define ZSKIP	5	/* skip file command, from receiver */
#define ZNAK	6	/* last packet was garbled */
#define ZABORT	7	/* abort */
#define ZFIN	8	/* finish session */
#define ZRPOS	9	/* resume file from this position, from receiver */
#define ZDATA	10	/* data packets to follow, from sender */
#define ZEOF	11	/* end of file, from sender */
#define ZFERR	12	/* fatal i/o error, from receiver */
#define ZCRC	13	/* request for file crc, from receiver */
#define ZCHALLENGE 14	/* "send this number back to me", from receiver */
#define ZCOMPL	15	/* request is complete */
#define ZCAN	16	/* other end cancelled with CAN-CAN-CAN-CAN-CAN */
#define ZFREECNT 17	/* request for free bytes on filesystem */
#define ZCOMMAND 18	/* command, from sending program */
#define ZSTDERR	19	/* output this message to stderr */


/* ZDLE escape sequences */


#define ZCRCE	'h'	/* CRC next, frame ends, header follows */
#define ZCRCG	'i'	/* CRC next, frame continues nonstop */
#define ZCRCQ	'j'	/* CRC next, send ZACK, frame continues nonstop */
#define ZCRCW	'k'	/* CRC next, send ZACK, frame ends */
#define ZRUB0	'l'	/* translate to 0177 */
#define ZRUB1	'm'	/* translate to 0377 */


/* ascii definitions */

#define SOH	1	/* ^A */
#define STX	2	/* ^B */
#define EOT	4	/* ^D */
#define ACK	6	/* ^F */
#define DLE	16	/* ^P */
#define XON	17	/* ^Q */
#define XOFF	19	/* ^S */
#define NAK	21	/* ^U */
#define SYN	22	/* ^V */
#define CAN	24	/* ^X */


/* state table entry.  There is one row of the table per
 * possible state.  Each row is a row of all reasonable
 * inputs for this state.  The entries are sorted so that
 * the most common inputs occur first, to reduce search time
 * Unexpected input is reported and ignored, as it might be
 * caused by echo or something.
 *
 * Extra ZRINIT headers are the receiver trying to resync.
 */
class FQTermConfig;

class FQTermZmodem;

class FQTermTelnet;

class FQTermFileDialog;

typedef int(FQTermZmodem:: *ActionFunc)(ZModem*);

typedef struct {
  int type; /* frame type */
  //	  int	(*func)(ZModem *) ;	/* transition function */
  ActionFunc func;
  int IFlush; /* flag: flush input first */
  int OFlush; /* flag: flush output first */
  ZMState newstate; /* new state.  May be overridden by func */
} StateTable;

//class FQTermTelnet;

class FQTermZmodem: public QObject {
  Q_OBJECT;
 public:
  FQTermZmodem(FQTermConfig *config, FQTermTelnet *netinterface, int type, int serverEncoding);
  ~FQTermZmodem();

  /* zmodem-supplied functions: */
  int ZmodemTInit(ZModem *info);
  int ZmodemTFile(char *file, char *rmtname, uint f0, uint f1, uint f2, uint f3,
                  int filesRem, int bytesRem, ZModem *info);
  int ZmodemTFinish(ZModem *info);
  int ZmodemAbort(ZModem *info);
  int ZmodemRInit(ZModem *info);
  int ZmodemRcv(uchar *str, int len, ZModem *info, int &consumed_bytes);
  int ZmodemAttention(ZModem *info);

  int ZmodemReset(ZModem *info);

  int YmodemTInit(ZModem *info);
  int XmodemTInit(ZModem *info);
  int YmodemRInit(ZModem *info);
  int XmodemRInit(ZModem *info);

  ulong FileCrc(char *name);
  const char *sname(ZModem*);
  const char *sname2(ZMState);

  /* caller-supplied functions: */
  int ZXmitChr(uchar c, ZModem *info);
  int ZXmitStr(const uchar *str, int len, ZModem *info);
  void ZIFlush(ZModem *info);
  void ZOFlush(ZModem *info);
  int ZAttn(ZModem *info);
  void ZStatus(int type, int value, const char *status);
  FILE *ZOpenFile(char *name, ulong crc, ZModem *info);

  int ZWriteFile(uchar *buffer, int len, FILE *, ZModem*);
  int ZCloseFile(ZModem *info);
  void ZFlowControl(int onoff, ZModem *info);

  /* end caller-supplied functions */

  int ZXmitHdr(int type, int format, const uchar data[4], ZModem *info);
  int ZXmitHdrHex(int type, const uchar data[4], ZModem *info);
  int ZXmitHdrBin(int type, const uchar data[4], ZModem *info);
  int ZXmitHdrBin32(int type, const uchar data[4], ZModem *info);
  uchar *putZdle(uchar *ptr, uchar c, ZModem *info);
  uchar *putHex(uchar *ptr, uchar c);

  uchar *ZEnc4(ulong n);
  ulong ZDec4(const uchar buf[4]);

  int YrcvChar(char c, register ZModem *info);
  int YrcvTimeout(register ZModem *info);
  void ZIdleStr(uchar *buffer, int len, ZModem *info);

  /* LEXICAL BOX: handle characters received from remote end.
   * These may be header, data or noise.
   *
   * This section is a finite state machine for parsing headers
   * and reading input data.  The info->chrCount field is effectively
   * the state variable.
   */

  int FinishChar(char c, register ZModem *info);
  int DataChar(uchar c, register ZModem *info);
  int HdrChar(uchar c, register ZModem *info);
  int IdleChar(uchar c, register ZModem *info);

  int YsendChar(char c, ZModem *info);

  int ZPF(ZModem *info);
  int Ignore(ZModem *info);
  int AnswerChallenge(register ZModem *info);
  int GotAbort(register ZModem *info);
  int GotCancel(ZModem *info);
  int GotCommand(ZModem *info);
  int GotStderr(register ZModem *info);
  int RetDone(ZModem *info);
  int GotCommandData(register ZModem *info, int crcGood);
  int GotStderrData(register ZModem *info, int crcGood);

  int GotFileName(ZModem *info, int crcGood);
  int ResendCrcReq(ZModem *info);
  int GotSinitData(ZModem *info, int crcGood);
  int ResendRpos(ZModem *info);
  int GotFileData(ZModem *info, int crcGood);
  int SendRinit(ZModem *info);

  int GotSinit(ZModem *info);
  int GotFile(ZModem *info);
  int GotFin(ZModem *info);
  int GotData(ZModem *info);
  int GotEof(ZModem *info);
  int GotFreecnt(ZModem *info);
  int GotFileCrc(ZModem *info);

  int GotRinit(ZModem*);
  int SendZSInit(ZModem*);
  int SendFileCrc(ZModem*);
  int GotSendAck(ZModem*);
  int GotSendDoneAck(ZModem*);
  int GotSendNak(ZModem*);
  int GotSendWaitAck(ZModem*);
  int SkipFile(ZModem*);
  int GotSendPos(ZModem*);

  int requestFile(ZModem *info, ulong crc);
  void parseFileName(ZModem *info, char *fileinfo);
  int fileError(ZModem *info, int type, int data);

  int ProcessPacket(ZModem *info);
  int rejectPacket(ZModem *info);
  int acceptPacket(ZModem *info);

  int calcCrc(uchar *str, int len);

  char *strdup(const char *str);

  int SendMoreFileData(ZModem *info);

  uint rcvHex(uint i, char c);

  int ZDataReceived(register ZModem *info, int crcGood);
  int dataSetup(register ZModem *info);
  int ZProtocol(register ZModem *info);

  int ZXmitData(int, int, uchar, uchar *data, ZModem*);
  int YXmitData(uchar *, int, ZModem*);

  int YSendFilename(ZModem*);
  int YSendData(ZModem*);
  int YSendFin(ZModem*);

  int sendFilename(ZModem*);
  int SendFileData(ZModem*);
  int ResendEof(ZModem*);

  int OverAndOut(ZModem*);

  int startFileData(ZModem *info);

  void zmodemlog(const char *fmt, ...);

  // interface function
  // all not completed
  void transferTimeOut(void*);
  void upload(char*);
  void uploadNext(char*);
  void transferFinish();
  void xferCancel();

  /* data received from remote, pass it to Zmodem funcs */
  void transferSendData(char *, int);
  void TransferCancel();

#if 1
  static StateTable RStartOps[];
  static StateTable RSinitWaitOps[];
  static StateTable RFileNameOps[];
  static StateTable RCrcOps[];
  static StateTable RFileOps[];
  static StateTable RDataOps[];
  static StateTable RFinishOps[];


  static StateTable TStartOps[];
  static StateTable TInitOps[];
  static StateTable FileWaitOps[];
  static StateTable CrcWaitOps[];
  static StateTable SendingOps[];
  static StateTable SendDoneOps[];
  static StateTable SendWaitOps[];
  static StateTable SendEofOps[];
  static StateTable TFinishOps[];

  static StateTable CommandDataOps[];
  static StateTable CommandWaitOps[];
  static StateTable StderrDataOps[];
  static StateTable DoneOps[];

  static StateTable *tables[];
  static const char *hdrnames[];
#endif

#if 0
  StateTable RStartOps[];
  StateTable RSinitWaitOps[];
  StateTable RFileNameOps[];
  StateTable RCrcOps[];
  StateTable RFileOps[];
  StateTable RDataOps[];
  StateTable RFinishOps[];


  StateTable TStartOps[];
  StateTable TInitOps[];
  StateTable FileWaitOps[];
  StateTable CrcWaitOps[];
  StateTable SendingOps[];
  StateTable SendDoneOps[];
  StateTable SendWaitOps[];
  StateTable SendEofOps[];
  StateTable TFinishOps[];

  StateTable CommandDataOps[];
  StateTable CommandWaitOps[];
  StateTable StderrDataOps[];
  StateTable DoneOps[];

  StateTable *tables[];
  char *hdrnames[];
#endif
  FQTermTelnet *telnet_;
  FQTermConfig *config_;
  //  Dialog
  //	QDialog *zmodemDialog, uploadListDialog;

  bool sending;

  enum_transferstate transferstate;

  ZModem info;

  //  connection type, e.g. telnet or ssh , 0=telnet, 1=ssh

  int connectionType;

  // Timer
  QTimer *zmodemTimer;

  //log file
  FILE *zmodemlogfile;

  int zerrno;
  uchar lastPullByte;

  QStringList strFileList;
  QStringList::Iterator itFile;

  int serverEncodingID;

  QString lastDownloadedFilename_;

  // end Member
 signals:
  void ZmodemState(int, int, const char *);

 public slots:
  void zmodemCancel();
  int ZmodemTimeout();
};

}  // namespace FQTerm

#endif  // FQTERM_ZMODEM_H
