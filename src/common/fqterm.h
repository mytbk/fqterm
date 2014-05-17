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

#ifndef FQTERM_GLOBAL_H
#define FQTERM_GLOBAL_H

#ifndef BUFSIZE
#define BUFSIZE (1024)
#endif

#include <QTextCodec>
#include <QString>
#include "fqterm_trace.h"

namespace FQTerm {

#if defined(WIN32) && (_MSC_VER >= 1400)
  //Using vc2005 or +
#define fq_strdup _strdup
#else
#define fq_strdup strdup
#endif

#ifdef FOUND_PYTHON
//#define HAVE_PYTHON
#endif //FOUND_PYTHON

//code convert
enum EncodingID {FQTERM_ENCODING_GBK = 0, FQTERM_ENCODING_BIG5 = 1, 
                 FQTERM_ENCODING_UTF8 = 2, FQTERM_ENCODING_HKSCS = 3, 
                 FQTERM_ENCODING_UAO = 4};

#if QT_VERSION >= 0X040700
#define GBK_CODECS_NAME "GB18030"
#else
#define GBK_CODECS_NAME "GBK"
#endif

#define G2U(s) ( QTextCodec::codecForName(GBK_CODECS_NAME)->toUnicode(s) )
#define U2G(s) ( QTextCodec::codecForName(GBK_CODECS_NAME)->fromUnicode(s) )
#define B2U(s) ( QTextCodec::codecForName("Big5")->toUnicode(s) )
#define U2B(s) ( QTextCodec::codecForName("Big5")->fromUnicode(s) )
#define H2U(s) ( QTextCodec::codecForName("Big5-HKSCS")->toUnicode(s) )
#define U2H(s) ( QTextCodec::codecForName("Big5-HKSCS")->fromUnicode(s) )
#define U2U8(s) ( s.toUtf8() )
#define U82U(s) ( QString::fromUtf8(s) )
#define A2U(s) (QTextCodec::codecForName("UAO")->toUnicode(s) )
#define U2A(s) ( QTextCodec::codecForName("UAO")->fromUnicode(s) )

inline QByteArray unicode2encoding(const QString &text, int encoding) {
  switch(encoding)
  {
  case FQTERM_ENCODING_GBK:
    return(U2G(text));
  case FQTERM_ENCODING_BIG5:
    return(U2B(text));
  case FQTERM_ENCODING_UTF8:
    return(U2U8(text));
  case FQTERM_ENCODING_HKSCS:
    return (U2H(text));
  case FQTERM_ENCODING_UAO:
    return (U2A(text));
  }

  return "";
}

inline QString encoding2unicode(const QByteArray &text, int encoding) {
  switch(encoding)
  {
  case FQTERM_ENCODING_GBK:
    return(G2U(text));
  case FQTERM_ENCODING_BIG5:
    return(B2U(text));
  case FQTERM_ENCODING_UTF8:
    return(U82U(text));
  case FQTERM_ENCODING_HKSCS:
    return (H2U(text));
  case FQTERM_ENCODING_UAO:
    return (A2U(text));
  }

  return "";
}

#define FQTERM_CTRL(c) ((c)&0x1f)

// every character has a 16-bit description
// low   8 for colors
// hight 8 for attributes
#define COLORMASK 0x00ff
#define ATTRMASK  0xff00

// color 8-bit
// bit 0-2 foreground
//  0   black
//  1   red
//  2   green
//  3   yellow
//  4   blue
//  5   magenta
//  6   cyan
//  7   white
// bit 3 highlight foreground
//  0   no
//  1   highlight
// bit 4-6 background
//  0   black
//  1   red
//  2   green
//  3   yellow
//  4   blue
//  5   magenta
//  6   cyan
//  7   white
// bit 7 highlight background (unused)
//  0   no
//  1   highlight

#define FGMASK 0x0f
#define BGMASK 0xf0
#define FG_HIGHLIGHT_MASK 0x08
#define BG_HIGHLIGHT_MASK 0x80

//set color
#define SETFG(c)    ( (c) & FGMASK )
#define SETBG(c)    ( ( (c) << 4 ) & BGMASK )
#define SET_FG_HIGHLIGHT(c) ( ( (c) << 3 ) & FG_HIGHLIGHT_MASK )
#define SET_BG_HIGHLIGHT(c) ( ( (c) << 7 ) & BG_HIGHLIGHT_MASK )

//get color
#define GETFG(c)    ( (c) & FGMASK )
#define GETBG(c)    ( ( (c) & BGMASK ) >> 4 )

#define GET_FG_HIGHLIGHT(c) ( ( (c) & FG_HIGHLIGHT_MASK ) >> 3 )
#define GET_BG_HIGHLIGHT(c) ( ( (c) & BG_HIGHLIGHT_MASK ) >> 7 )

//#define GET_INVERSE_COLOR(c)  ( SETFG( (7 - (c & 0x7)) | (c & 0x08) ) | SETBG(15-GETBG(c)) )
#define GET_INVERSE_COLOR(c)  ( SETFG(15-GETFG(c)) | SETBG(15-GETBG(c)) )

// mask for attr
#define BRIGHTMASK  0x01
#define DIMMASK     0x02
#define UNDERLINEMASK   0x08
#define BLINKMASK   0x10
#define RAPIDBLINKMASK  0x20
#define REVERSEMASK 0x40
#define INVISIBLEMASK   0x80

//set attributes
#define SETBRIGHT(a)    ( (a) | BRIGHTMASK )
#define SETDIM(a)   ( (a) | DIMMASK )
#define SETUNDERLINE(a) ( (a) | UNDERLINEMASK )
#define SETBLINK(a)     ( (a) | BLINKMASK )
#define SETRAPIDBLINK(a) ( (a) | RAPIDBLINKMASK )
#define SETREVERSE(a)   ( (a) | REVERSEMASK )
#define SETINVISIBLE(a) ( (a) | INVISIBLEMASK )

//get attributes
#define GETBRIGHT(a)    ( (a) & BRIGHTMASK )
#define GETDIM(a)   ( (a) & DIMMASK )
#define GETUNDERLINE(a) ( (a) & UNDERLINEMASK )
#define GETBLINK(a)     ( (a) & BLINKMASK )
#define GETRAPIDBLINK(a) ( (a) & RAPIDBLINKMASK )
#define GETREVERSE(a)   ( (a) & REVERSEMASK )
#define GETINVISIBLE(a) ( (a) & INVISIBLEMASK )

//default color
#define NO_COLOR ( SETFG(7) | SETBG(0) )
//default attribute
#define NO_ATTR 0x04

// other definations
#ifndef NULL
#define NULL 0
#endif

#define DAE_FINISH  10001
#define DAE_TIMEOUT 10002

#define PYE_ERROR   10003
#define PYE_FINISH  10004

// some keys
#define CHAR_NUL    0x00    // ^@
#define CHAR_ENQ    0x05    // ^E
#define CHAR_BELL   0x07    // ^G
#define CHAR_BS     0x08    // ^H
#define CHAR_TAB    0x09    // ^I
#define CHAR_LF     0x0a    // ^J
#define CHAR_VT     0x0b    // ^K
#define CHAR_FF     0x0c    // ^L
#define CHAR_CR     0x0d    // ^M
#define CHAR_SO     0x0e    // ^N
#define CHAR_SI     0x0f    // ^O
#define CHAR_CAN    0x18    // ^X
#define CHAR_ESC    0x1b    // ^[
#define CHAR_DEL    0x7f  // DEL

#define CHAR_NORMAL -1

const char ANSWERBACK_MESSAGE[] = "This is fqterm.";

// telnet state
#define TSRESOLVING     30
#define TSHOSTFOUND     31
#define TSHOSTNOTFOUND  32
#define TSCONNECTING    33
#define TSHOSTCONNECTED 34
#define TSPROXYCONNECTED    35
#define TSPROXYAUTH     36
#define TSPROXYFAIL     38
#define TSREFUSED       39
#define TSREADERROR     40
#define TSCLOSED        41
#define TSCLOSEFINISH   42
#define TSCONNECTVIAPROXY   43
#define TSEGETHOSTBYNAME    44
#define TSEINIWINSOCK   45
#define TSERROR         46
#define TSPROXYERROR    47
#define TSWRITED        48

// zmodem types
#define RcvByteCount    0   /* value is # bytes received */
#define SndByteCount    1   /* value is # bytes sent */
#define RcvTimeout  2   /* receiver did not respond, aborting */
#define SndTimeout  3   /* value is # of consecutive send timeouts */
#define RmtCancel   4   /* remote end has cancelled */
#define ProtocolErr 5   /* protocol error has occurred, val=hdr */
#define RemoteMessage   6   /* message from remote end */
#define DataErr     7   /* data error, val=error count */
#define FileErr     8   /* error writing file, val=errno */
#define FileBegin   9   /* file transfer begins, str=name */
#define FileEnd     10  /* file transfer ends, str=name */
#define FileSkip    11  /* file being skipped, str=name */


// proxy type
#define NOPROXY         0
#define WINGATE         1
#define SOCKS4          2
#define SOCKS5          3
#define HTTP            4

// UI ID
#define ID_FILE_CONNECT     0x00
#define ID_FILE_DISCONNECT  0x01
#define ID_FILE_ADDRESS     0x02
#define ID_FILE_QUICK       0x03
#define ID_FILE_EXIT        0x04

#define ID_EDIT_COPY        0x10
#define ID_EDIT_PASTE       0x11
#define ID_EDIT_COLOR       0x12
#define ID_EDIT_RECT        0x13
#define ID_EDIT_AUTO        0x14
#define ID_EDIT_WW          0x15

#define ID_EDIT_ESC_NO      0x17
#define ID_EDIT_ESC_ESC     0x18
#define ID_EDIT_ESC_U       0x19
#define ID_EDIT_ESC_CUS     0x1a

#define ID_EDIT_CODEC_GBK   0x1b
#define ID_EDIT_CODEC_BIG5  0x1c


#define ID_VIEW_FONT        0x20
#define ID_VIEW_COLOR       0x21
#define ID_VIEW_REFRESH     0x22
#define ID_VIEW_LANG        0x23
#define ID_VIEW_FULL        0x24
#define ID_VIEW_BOSS        0x25
#define ID_VIEW_SCROLL_HIDE 0x26
#define ID_VIEW_SCROLL_LEFT 0x27
#define ID_VIEW_SCROLL_RIGHT 0x28
#define ID_VIEW_SWITCH      0x29
#define ID_VIEW_STATUS      0x3a

#define ID_OPTION_CURRENT   0x30
#define ID_OPTION_DEFAULT   0x31
#define ID_OPTION_PREF      0x32

#define ID_SPEC_ANTI        0x40
#define ID_SPEC_AUTO        0x41
#define ID_SPEC_MESSAGE     0x42
#define ID_SPEC_BEEP        0x43
#define ID_SPEC_MOUSE       0x44
#define ID_SPEC_ARTICLE     0x45

#define ID_SCRIPT_RUN       0x50
#define ID_SCRIPT_STOP      0x51

#define ID_HELP_ABOUT       0x60
#define ID_HELP_HOMEPAGE    0x61

//for open last addr
#define LAST_ADDRESS_INDEX  -321

//script function names:
#define SFN_ANTI_IDLE       "antiIdle"
#define SFN_ON_BELL         "onBell"
#define SFN_AUTO_REPLY      "autoReply"
#define SFN_DATA_EVENT      "dataEvent"
#define SFN_KEY_EVENT       "keyEvent"
#define SFN_MOUSE_EVENT     "mouseEvent"
#define SFN_DETECT_MENU     "detectMenu"

//script key event type
#define SKET_KEY_PRESS      0

//script mouse event type
#define SMET_UNKOWN         -1
#define SMET_MOUSE_PRESS    0
#define SMET_MOUSE_RELEASE  1
#define SMET_MOUSE_MOVE     2
#define SMET_MOUSE_DBCLICK  3
#define SMET_WHEEL          4

//script button state & key modifier
#define SBS_LEFT_BUTTON     0x01
#define SBS_RIGHT_BUTTON    0x02
#define SBS_MID_BUTTON      0x04
#define SBS_ALT             0x08
#define SBS_CTRL            0x10
#define SBS_SHIFT           0x20

}  // namespace FQTerm

#endif  // FQTERM_GLOBAL_H
