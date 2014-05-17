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

#include <QApplication>
#include <QString>
#include <QFileDialog>
#include <QFileInfo>

#include "fqterm.h"
#include "fqterm_config.h"
#include "fqterm_param.h"
#include "fqterm_path.h"
#include "fqterm_telnet.h"
#include "fqterm_zmodem.h"
#include "fqterm_filedialog.h"

#ifdef FQTERM_ZMODEM_DEBUG
#include <sys/time.h>
#endif

namespace FQTerm {

static const uchar zeros[4] =	 {
  0, 0, 0, 0
};
 
static const char hexChars[] = "0123456789abcdef";
 
static const uchar AckStr[1] =  {
  ACK
};
static const uchar NakStr[1] =  {
  NAK
};
static const uchar CanStr[2] =  {
  CAN, CAN
};
static const uchar eotstr[1] =  {
  EOT
};


/*
 *  Crc calculation stuff
 */

/* crctab calculated by Mark G. Mendel, Network Systems Corporation */
unsigned short crctab[256] =  {
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129,
  0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210, 0x3273,
  0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a,
  0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401, 0x64e6,
  0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf,
  0xc5ac, 0xd58d, 0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695,
  0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
  0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc,
  0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5, 0x4ad4,
  0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf,
  0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4, 0x5cc5,
  0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a,
  0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32,
  0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59,
  0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
  0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067, 0x83b9,
  0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290,
  0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb, 0x95a8,
  0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481,
  0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f,
  0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676,
  0x4615, 0x5634, 0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a,
  0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
  0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75,
  0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e, 0xed0f,
  0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64,
  0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f, 0xff3e, 0xcf5d, 0xdf7c,
  0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93,
  0x3eb2, 0x0ed1, 0x1ef0
};

/*
 * updcrc macro derived from article Copyright (C) 1986 Stephen Satchell.
 *  NOTE: First argument must be in range 0 to 255.
 *        Second argument is referenced twice.
 *
 * Programmers may incorporate any or all code into their programs,
 * giving proper credit within the source. Publication of the
 * source routines is permitted so long as proper credit is given
 * to Stephen Satchell, Satchell Evaluations and Chuck Forsberg,
 * Omen Technology.
 */

#define updcrc(cp, crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)

/*
 * Copyright (C) 1986 Gary S. Brown.  You may use this program, or
 * code or tables extracted from it, as desired without restriction.
 */

/* First, the polynomial itself and its table of feedback terms.  The  */
/* polynomial is                                                       */
/* X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0 */
/* Note that we take it "backwards" and put the highest-order term in  */
/* the lowest-order bit.  The X^32 term is "implied"; the LSB is the   */
/* X^31 term, etc.  The X^0 term (usually shown as "+1") results in    */
/* the MSB being 1.                                                    */

/* Note that the usual hardware shift register implementation, which   */
/* is what we're using (we're merely optimizing it by doing eight-bit  */
/* chunks at a time) shifts bits into the lowest-order term.  In our   */
/* implementation, that means shifting towards the right.  Why do we   */
/* do it this way?  Because the calculated CRC must be transmitted in  */
/* order from highest-order term to lowest-order term.  UARTs transmit */
/* characters in order from LSB to MSB.  By storing the CRC this way,  */
/* we hand it to the UART in the order low-byte to high-byte; the UART */
/* sends each low-bit to hight-bit; and the result is transmission bit */
/* by bit from highest- to lowest-order term without requiring any bit */
/* shuffling on our part.  Reception works similarly.                  */

/* The feedback terms table consists of 256, 32-bit entries.  Notes:   */
/*                                                                     */
/*     The table can be generated at runtime if desired; code to do so */
/*     is shown later.  It might not be obvious, but the feedback      */
/*     terms simply represent the results of eight shift/xor opera-    */
/*     tions for all combinations of data and CRC register values.     */
/*                                                                     */
/*     The values must be right-shifted by eight bits by the "updcrc"  */
/*     logic; the shift must be unsigned (bring in zeroes).  On some   */
/*     hardware you could probably optimize the shift in assembler by  */
/*     using byte-swap instructions.                                   */

unsigned long cr3tab[] =  {
  /* CRC polynomial 0xedb88320 */
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
      0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
      0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
      0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
      0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
      0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
      0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
      0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
      0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
      0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
      0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
      0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
      0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
      0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
      0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
      0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
      0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
      0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
      0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
      0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
      0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
      0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
      0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
      0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
      0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
      0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
      0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
      0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
      0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
      0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
      0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
      0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
      0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
      0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
      0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
      0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
      0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
      0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
      0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
      0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
      0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
      0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
      0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
      };

#define UPDC32(b, c) (cr3tab[((int)c ^ b) & 0xff] ^ ((c >> 8) & 0x00FFFFFF))

#define updcrc(cp, crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)



#if 1
StateTable FQTermZmodem::RStartOps[] =  {
  {ZSINIT, &FQTermZmodem::GotSinit, 0, 1, RSinitWait},
      /* SINIT, wait for attn str */
  {ZFILE, &FQTermZmodem::GotFile, 0, 0, RFileName},
      /* FILE, wait for filename */
  {ZRQINIT, &FQTermZmodem::SendRinit, 0, 1, RStart},
      /* sender confused, resend */
  {ZFIN, &FQTermZmodem::GotFin, 1, 0, RFinish},
      /* sender shutting down */
  {ZNAK, &FQTermZmodem::SendRinit, 1, 0, RStart},
      /* RINIT was bad, resend */
  {ZFREECNT, &FQTermZmodem::GotFreecnt, 0, 0, RStart},
      /* sender wants free space */
  {ZCOMMAND, &FQTermZmodem::GotCommand, 0, 0, CommandData},
      /* sender wants command */
  {ZSTDERR, &FQTermZmodem::GotStderr, 0, 0, StderrData},
      /* sender wants to send msg */
  {ZRINIT, &FQTermZmodem::ZmodemTInit, 1, 1, TStart},
  {99, &FQTermZmodem::ZPF, 0, 0, RStart}
  /* anything else is an error */
};

StateTable FQTermZmodem::RSinitWaitOps[] =  {
  /* waiting for data */
  {
    99, &FQTermZmodem::ZPF, 0, 0, RSinitWait
  },
};

StateTable FQTermZmodem::RFileNameOps[] =  {
  /* waiting for file name */
  {
    99, &FQTermZmodem::ZPF, 0, 0, RFileName
  },
};

StateTable FQTermZmodem::RCrcOps[] =  {
  /* waiting for CRC */
  {
    ZCRC, &FQTermZmodem::GotFileCrc, 0, 0, RFile
  }, { /* sender sent it */
    ZNAK, &FQTermZmodem::ResendCrcReq, 0, 0, RCrc
  }, { /* ZCRC was bad, resend */
    ZRQINIT, &FQTermZmodem::SendRinit, 1, 1, RStart
  }, { /* sender confused, restart */
    ZFIN, &FQTermZmodem::GotFin, 1, 1, RFinish
  }, { /* sender signing off */
    99, &FQTermZmodem::ZPF, 0, 0, RCrc
  },
};


StateTable FQTermZmodem::RFileOps[] =  {
  /* waiting for ZDATA */
  {
    ZDATA, &FQTermZmodem::GotData, 0, 0, RData
  }, { /* got it */
    ZNAK, &FQTermZmodem::ResendRpos, 0, 0, RFile
  }, { /* ZRPOS was bad, resend */
    ZEOF, &FQTermZmodem::GotEof, 0, 0, RStart
  }, { /* end of file */
    ZRQINIT, &FQTermZmodem::SendRinit, 1, 1, RStart
  }, { /* sender confused, restart */
    ZFILE, &FQTermZmodem::ResendRpos, 0, 0, RFile
  }, { /* ZRPOS was bad, resend */
    ZFIN, &FQTermZmodem::GotFin, 1, 1, RFinish
  }, { /* sender signing off */
    99, &FQTermZmodem::ZPF, 0, 0, RFile
  },
};

/* waiting for data, but a packet could possibly arrive due
 * to error recovery or something
 */
StateTable FQTermZmodem::RDataOps[] =  {
  {
    ZRQINIT, &FQTermZmodem::SendRinit, 1, 1, RStart
  }, { /* sender confused, restart */
    ZFILE, &FQTermZmodem::GotFile, 0, 1, RFileName
  }, { /* start a new file (??) */
    ZNAK, &FQTermZmodem::ResendRpos, 1, 1, RFile
  }, { /* ZRPOS was bad, resend */
    ZFIN, &FQTermZmodem::GotFin, 1, 1, RFinish
  }, { /* sender signing off */
    ZDATA, &FQTermZmodem::GotData, 0, 1, RData
  }, { /* file data follows */
    ZEOF, &FQTermZmodem::GotEof, 1, 1, RStart
  }, { /* end of file */
    99, &FQTermZmodem::ZPF, 0, 0, RData
  },
};

/* here if we've sent ZFERR or ZABORT.  Waiting for ZFIN */

StateTable FQTermZmodem::RFinishOps[] =  {

  {
    ZRQINIT, &FQTermZmodem::SendRinit, 1, 1, RStart
  } , { /* sender confused, restart */
    ZFILE, &FQTermZmodem::GotFile, 1, 1, RFileName
  }, { /* start a new file */
    ZNAK, &FQTermZmodem::GotFin, 1, 1, RFinish
  }, { /* resend ZFIN */
    ZFIN, &FQTermZmodem::GotFin, 1, 1, RFinish
  }, { /* sender signing off */
    99, &FQTermZmodem::ZPF, 0, 0, RFinish
  },
};


/* sent ZRQINIT, waiting for response */
StateTable FQTermZmodem::TStartOps[] =  {
  {
    ZRINIT, &FQTermZmodem::GotRinit, 1, 1, TStart
  }, {
    ZCHALLENGE, &FQTermZmodem::AnswerChallenge, 1, 0, TStart
  }, {
    ZABORT, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZFERR, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZNAK, &FQTermZmodem::Ignore, 0, 0, TStart
  }, {
    ZCOMMAND, &FQTermZmodem::GotCommand, 0, 0, CommandData
  }, {
    ZSTDERR, &FQTermZmodem::GotStderr, 0, 0, StderrData
  }, {
    99, &FQTermZmodem::ZPF, 0, 0, TStart
  },
};

/* sent ZSINIT, waiting for response */
StateTable FQTermZmodem::TInitOps[] =  {
  {
    ZACK, &FQTermZmodem::RetDone, 1, 0, TInit
  }, {
    ZNAK, &FQTermZmodem::SendZSInit, 1, 0, TInit
  }, {
    ZRINIT, &FQTermZmodem::GotRinit, 1, 1, TInit
  }, { /* redundant, but who cares */
    ZCHALLENGE, &FQTermZmodem::AnswerChallenge, 1, 0, TInit
  }, {
    ZABORT, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZFERR, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZCOMMAND, &FQTermZmodem::GotCommand, 0, 0, CommandData
  }, {
    ZSTDERR, &FQTermZmodem::GotStderr, 0, 0, StderrData
  }, {
    99, &FQTermZmodem::ZPF, 0, 0, TInit
  },
};

/* sent ZFILE, waiting for response */
StateTable FQTermZmodem::FileWaitOps[] =  {
  {
    ZRPOS, &FQTermZmodem::SendFileData, 1, 1, Sending
  }, {
    ZSKIP, &FQTermZmodem::SkipFile, 1, 0, TStart
  }, {
    ZCRC, &FQTermZmodem::SendFileCrc, 1, 0, FileWait
  }, {
    ZNAK, &FQTermZmodem::sendFilename, 1, 0, FileWait
  }, {
    ZRINIT, &FQTermZmodem::sendFilename, 1, 1, FileWait
  }, { /* rcvr confused, retry file */
    ZABORT, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZFERR, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZCHALLENGE, &FQTermZmodem::AnswerChallenge, 1, 0, FileWait
  }, {
    ZCOMMAND, &FQTermZmodem::GotCommand, 0, 0, CommandData
  }, {
    ZSTDERR, &FQTermZmodem::GotStderr, 0, 0, StderrData
  }, {
    ZACK, &FQTermZmodem::SendFileData, 1, 0, Sending
  }, { // receiver always sends ZACK back
    99, &FQTermZmodem::ZPF, 0, 0, FileWait
  },
};

/* sent file CRC, waiting for response */
StateTable FQTermZmodem::CrcWaitOps[] =  {
  {
    ZRPOS, &FQTermZmodem::SendFileData, 1, 0, Sending
  }, {
    ZSKIP, &FQTermZmodem::SkipFile, 1, 0, FileWait
  }, {
    ZNAK, &FQTermZmodem::SendFileCrc, 1, 0, CrcWait
  }, {
    ZRINIT, &FQTermZmodem::sendFilename, 1, 1, FileWait
  }, { /* rcvr confused, retry file */
    ZABORT, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZFERR, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZCRC, &FQTermZmodem::SendFileCrc, 0, 0, CrcWait
  }, {
    ZCHALLENGE, &FQTermZmodem::AnswerChallenge, 0, 0, CrcWait
  }, {
    ZCOMMAND, &FQTermZmodem::GotCommand, 0, 0, CommandData
  }, {
    ZSTDERR, &FQTermZmodem::GotStderr, 0, 0, StderrData
  }, {
    99, &FQTermZmodem::ZPF, 0, 0, CrcWait
  },
};

/* sending data, interruptable */
StateTable FQTermZmodem::SendingOps[] =  {
  {
    ZACK, &FQTermZmodem::GotSendAck, 0, 0, Sending
  }, {
    ZRPOS, &FQTermZmodem::GotSendPos, 1, 1, Sending
  }, {
    ZSKIP, &FQTermZmodem::SkipFile, 1, 1, FileWait
  }, {
    ZNAK, &FQTermZmodem::GotSendNak, 1, 1, Sending
  }, {
    ZRINIT, &FQTermZmodem::sendFilename, 1, 1, FileWait
  },{ /* rcvr confused, retry file */
    ZABORT, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZFERR, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    99, &FQTermZmodem::ZPF, 0, 0, SendWait
  },
};

/* sent data, need to send EOF */
StateTable FQTermZmodem::SendDoneOps[] =  {
  {
    ZACK, &FQTermZmodem::GotSendDoneAck, 0, 0, SendWait
  }, {
    ZRPOS, &FQTermZmodem::GotSendPos, 1, 1, Sending
  }, {
    ZSKIP, &FQTermZmodem::SkipFile, 1, 1, FileWait
  }, {
    ZNAK, &FQTermZmodem::GotSendNak, 1, 1, Sending
  }, {
    ZRINIT, &FQTermZmodem::sendFilename, 1, 1, FileWait
  }, { /* rcvr confused, retry file */
    ZABORT, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZFERR, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    99, &FQTermZmodem::ZPF, 0, 0, SendWait
  },
};

/* sending data, waiting for ACK */
StateTable FQTermZmodem::SendWaitOps[] =  {
  {
    ZACK, &FQTermZmodem::GotSendWaitAck, 0, 0, Sending
  }, {
    ZRPOS, &FQTermZmodem::GotSendPos, 0, 0, SendWait
  }, {
    ZSKIP, &FQTermZmodem::SkipFile, 1, 1, FileWait
  }, {
    ZNAK, &FQTermZmodem::GotSendNak, 0, 0, Sending
  }, {
    ZRINIT, &FQTermZmodem::sendFilename, 1, 1, FileWait
  }, { /* rcvr confused, retry file */
    ZABORT, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZFERR, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    99, &FQTermZmodem::ZPF, 0, 0, SendWait
  },
};

/* sent ZEOF, waiting for new RINIT */
StateTable FQTermZmodem::SendEofOps[] =  {
  {
    ZRINIT, &FQTermZmodem::SkipFile, 1, 0, TFinish
  }, { /* successful completion */
    ZACK, &FQTermZmodem::Ignore, 0, 0, SendEof
  }, { /* probably ACK from last packet */
    ZRPOS, &FQTermZmodem::GotSendPos, 1, 1, SendWait
  }, {
    ZSKIP, &FQTermZmodem::SkipFile, 1, 1, TStart
  }, {
    ZNAK, &FQTermZmodem::ResendEof, 1, 0, SendEof
  }, {
    ZRINIT, &FQTermZmodem::sendFilename, 1, 1, FileWait
  }, { /* rcvr confused, retry file */
    ZABORT, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZFERR, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    99, &FQTermZmodem::ZPF, 0, 0, SendEof
  },
};

StateTable FQTermZmodem::TFinishOps[] =  {
  {
    ZFIN, &FQTermZmodem::OverAndOut, 1, 1, Done
  }, {
    ZNAK, &FQTermZmodem::ZmodemTFinish, 1, 1, TFinish
  }, {
    ZRINIT, &FQTermZmodem::ZmodemTFinish, 1, 1, TFinish
  }, {
    ZABORT, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    ZFERR, &FQTermZmodem::GotAbort, 1, 1, TFinish
  }, {
    99, &FQTermZmodem::ZPF, 0, 0, TFinish
  },
};

StateTable FQTermZmodem::CommandDataOps[] =  {
  {
    99, &FQTermZmodem::ZPF, 0, 0, CommandData
  },
};

StateTable FQTermZmodem::CommandWaitOps[] =  {
  {
    99, &FQTermZmodem::ZPF, 0, 0, CommandWait
  },
};

StateTable FQTermZmodem::StderrDataOps[] =  {
  {
    99, &FQTermZmodem::ZPF, 0, 0, StderrData
  },
};

StateTable FQTermZmodem::DoneOps[] =  {
  {
    99, &FQTermZmodem::ZPF, 0, 0, Done
  },
};

StateTable *FQTermZmodem::tables[] =  {
  FQTermZmodem::RStartOps, FQTermZmodem::RSinitWaitOps, FQTermZmodem::RFileNameOps,
  FQTermZmodem::RCrcOps, FQTermZmodem::RFileOps, FQTermZmodem::RDataOps,
  FQTermZmodem::RDataOps,  /* RDataErr is the same as RData */
  FQTermZmodem::RFinishOps,

  FQTermZmodem::TStartOps, FQTermZmodem::TInitOps, FQTermZmodem::FileWaitOps,
  FQTermZmodem::CrcWaitOps, FQTermZmodem::SendingOps, FQTermZmodem::SendWaitOps,
  FQTermZmodem::SendDoneOps, FQTermZmodem::SendEofOps, FQTermZmodem::TFinishOps,

  FQTermZmodem::CommandDataOps, FQTermZmodem::CommandWaitOps,
  FQTermZmodem::StderrDataOps, FQTermZmodem::DoneOps,
};

const char *FQTermZmodem::hdrnames[] =  {
  "ZRQINIT", "ZRINIT", "ZSINIT", "ZACK", "ZFILE", "ZSKIP", "ZNAK", "ZABORT",
  "ZFIN", "ZRPOS", "ZDATA", "ZEOF", "ZFERR", "ZCRC", "ZCHALLENGE", "ZCOMPL",
  "ZCAN", "ZFREECNT", "ZCOMMAND", "ZSTDERR",
};

#endif

FQTermZmodem::FQTermZmodem(FQTermConfig *config, FQTermTelnet *netinterface, int type, int serverEncoding) {

  //now set network interface Telnet

  connectionType = type;

  serverEncodingID = serverEncoding;

  switch (connectionType) {
	case 0:
	case 1:
 	case 2:
	  telnet_ = netinterface;
	  break;
	default:
	  FQ_TRACE("zmodem", 0) << "connection type unknown! Expect segmentation fault!";
	  telnet_ =  netinterface;
	  break;
  }

  config_ = config;

  sending = false;

  transferstate = notransfer;

  zmodemTimer = new QTimer(this);
  FQ_VERIFY(connect(zmodemTimer, SIGNAL(timeout()), this, SLOT(ZmodemTimeout())));

#ifdef FQTERM_ZMODEM_DEBUG
  zmodemlogfile = fopen("zmodem.log", "w+");
  fprintf(zmodemlogfile, "%s", "\n================================\n");
  fclose(zmodemlogfile);
#endif


  //init struct INFO
  info.zrinitflags = CANFDX | CANOVIO | CANBRK | CANFC32;
  info.zsinitflags = 0;
  info.attn = NULL;
  info.bufsize = 0; /* full streaming */
  info.buffer = NULL;

  zerrno = 0;
  lastPullByte = 0;

  ZmodemReset(&info);
  // other init function not complete

}

FQTermZmodem::~FQTermZmodem(){}

int FQTermZmodem::ZmodemTInit(ZModem *info) {
  int err;
  int i;

  info->state = TStart;
  info->Protocol = ZMODEM;
  info->crc32 = 0;
  info->packetCount = 0;
  info->errCount = 0;
  info->escCtrl = info->escHibit = info->atSign = info->escape = 0;
  info->InputState = Idle;
  info->canCount = info->chrCount = 0;
  info->windowCount = 0;
  info->filename = NULL;
  info->bufsize = 0;
  info->interrupt = 0;
  info->waitflag = 0;

  //	if( info->packetsize <= 0 )
  info->packetsize = 8096;
  info->windowsize = 0;
  //we won't be receiving much data, pick a reasonable buffer
  // size (largest packet will do)
  //

  i = info->packetsize *2;

  if (info->buffer != NULL) {
    free(info->buffer);
    info->buffer = NULL;
  }
  //since in the constructor function buffer is malloc

  if (i < 1024) {
    i = 1024;
  }
  info->buffer = (uchar*)malloc(i);

  ZIFlush(info);

//  FQTermConfig *config = new FQTermConfig(getPath(USER_CONFIG) + "fqterm.cfg");
//  QString strPrevSave = config->getItemValue("global", "previous");

//  if (strPrevSave.isEmpty()) {
//	strFileList = QFileDialog::getOpenFileNames(0, "Choose the files",
//											getPath(USER_CONFIG), "All files(*)");
//  } else {
//	strFileList = QFileDialog::getOpenFileNames(0, "Choose the files",
//											strPrevSave, "All files(*)");
//  }
  FQTermFileDialog fileDialog(config_);
  strFileList = fileDialog.getOpenNames("Choose a file to upload", "");
  if (strFileList.count() != 0) {
    QStringList::Iterator itFile = strFileList.begin();
    QFileInfo fi(*itFile);
  }

  this->transferstate = transferstop;

  // optional: send "rz\r" to remote end 
  if (DoInitRZ) {
    if ((err = ZXmitStr((uchar*)"rz\r", 3, info))) {
      return err;
    }
  }

  if ((err = ZXmitHdr(ZRQINIT, ZHEX, zeros, info))) {
    // nudge receiver 
    return err;
  }

  info->timeout = 60;

  zmodemlog("ZmodemTInit[%s]: sent ZRQINIT\n", sname(info));

  return 0;
}


int FQTermZmodem::ZmodemTFile(char *file, char *rfile, uint f0, uint f1, uint f2,
                              uint f3, int filesRem, int bytesRem, ZModem *info) {
  if (file == NULL || (info->file = fopen(file, "rb")) == NULL) {
    return ZmErrCantOpen;
  }

  info->fileEof = 0;
  info->filename = file;
  if (rfile != NULL) {
    info->rfilename = rfile;
  } else {
    info->rfilename = strdup("noname");
  }
  info->filesRem = filesRem;
  info->bytesRem = bytesRem;
  info->fileFlags[3] = f0;
  info->fileFlags[2] = f1;
  info->fileFlags[1] = f2;
  info->fileFlags[0] = f3;
  info->offset = info->lastOffset = 0;
  info->len = info->date = info->fileType = info->mode = 0;
  if (info->filename != NULL) {
    struct stat buf;
    if (stat(info->filename, &buf) == 0) {
      info->len = buf.st_size;
      info->date = buf.st_mtime;
      info->fileType = 0;
      info->mode = (buf.st_mode &0777) | 0100000;
    }
  }

  if (info->Protocol == XMODEM) {
    return YSendData(info);
  }

  if (info->Protocol == YMODEM) {
    return YSendFilename(info);
  }

  info->state = FileWait;
  ZStatus(FileBegin, info->bytesRem, info->rfilename);
  zmodemlog("ZmodemTFile[%s]: send ZFILE(%s)\n", sname(info), info->rfilename);
  return sendFilename(info);
}


int FQTermZmodem::ZmodemTFinish(ZModem *info) {
  int i;
  if (info->Protocol == XMODEM) {
    return ZmDone;
  }

  if (info->Protocol == YMODEM) {
    return YSendFin(info);
  }

  info->state = TFinish;
  if (info->buffer != NULL) {
    free(info->buffer);
    info->buffer = NULL;
  }
  zmodemlog("ZmodemTFinish[%s]: send ZFIN\n", sname(info));
  i = ZXmitHdr(ZFIN, ZHEX, zeros, info);

  return i;
}



int FQTermZmodem::ZmodemAbort(ZModem *info) {
  uchar canistr[] =  {
    CAN, CAN, CAN, CAN, CAN, CAN, CAN, CAN, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0
  };
  info->state = Done;
  ZIFlush(info);
  ZOFlush(info);

  transferstate = transferstop; // transfer complete
  ZmodemReset(info); //Tranfer complete, zmodem return to receive state

  zmodemlog("ZmodemAbort[%s]: send CAN\n", sname(info));
  return ZXmitStr(canistr, sizeof(canistr), info);
}


int FQTermZmodem::ZmodemRInit(ZModem *info) {
  info->packetCount = 0;
  info->offset = 0;
  info->errCount = 0;
  info->escCtrl = info->escHibit = info->atSign = info->escape = 0;
  info->InputState = Idle;
  info->canCount = info->chrCount = 0;
  info->filename = NULL;
  info->interrupt = 0;
  info->waitflag = 0;
  info->attn = NULL;
  info->file = NULL;

  if (info->buffer != NULL) {
    free(info->buffer);
    info->buffer = NULL;
  }

  info->buffer = (uchar*)malloc(1024);

  info->state = RStart;
  info->timeoutCount = 0;

  ZIFlush(info);

  /* Don't send ZRINIT right away, there might be a ZRQINIT in
   * the input buffer.  Instead, set timeout to zero and return.
   * This will allow ZmodemRcv() to check the input stream first.
   * If nothing found, a ZRINIT will be sent immediately.
   */
  info->timeout = 0;

  zmodemlog("ZmodemRInit[%s]: flush input, new state = RStart\n", sname(info));

  return 0;
}

int FQTermZmodem::ZmodemRcv(uchar *str, int len, ZModem *info,
                            int &consumed_bytes) {
  uchar c;
  int err;

  zmodemlog("zmodemRcv called");

  info->rcvlen = len;

  while (--info->rcvlen >= 0) {
    c =  *str++;

    if (c == CAN) {
      if (++info->canCount >= 5) {
        ZStatus(RmtCancel, 0, NULL);
        consumed_bytes = len - info->rcvlen - 1;
        return ZmErrCancel;
      }
    } else {
      info->canCount = 0;
    }

    if (info->InputState == Ysend) {
      if ((err = YsendChar(c, info))) {
        consumed_bytes = len - info->rcvlen - 1;
        return err;
      }
    } else if (info->InputState == Yrcv) {
      if ((err = YrcvChar(c, info))) {
        consumed_bytes = len - info->rcvlen - 1;
        return err;
      }
    } else if (c != XON && c != XOFF) {
      /* now look at what we have */
      switch (info->InputState) {
        case Idle:
          if ((err = IdleChar(c, info))) {
            consumed_bytes = len - info->rcvlen - 1;
            return err;
          }
          break;
        case Inhdr:
          if ((err = HdrChar(c, info))) {
            consumed_bytes = len - info->rcvlen - 1;            
            return err;
          }
          break;
        case Indata:
          if ((err = DataChar(c, info))) {
            consumed_bytes = len - info->rcvlen - 1;            
            return err;
          }
          break;
        case Finish:
          if ((err = FinishChar(c, info))) {
            consumed_bytes = len - info->rcvlen - 1;            
            return err;
          }
          break;
        default:
          break;
      }
    }
  }
  
  consumed_bytes = len - info->rcvlen - 1;
  return 0;
}




int FQTermZmodem::ZmodemTimeout( /*ZModem *info*/) {
  /* timed out while waiting for input */

  ++info.timeoutCount;

  zmodemlog("timeout %d [%s]\n", info.timeoutCount, sname(&info));

  switch (info.state) {
    /* receive */
    case RStart:
      /* waiting for INIT frame from other end */
      if (info.timeoutCount > 4) {
        return YmodemRInit(&info);
      }

    case RSinitWait:
    case RFileName:
      if (info.timeout > 0) {
        ZStatus(SndTimeout, info.timeoutCount, NULL);
      }
      if (info.timeoutCount > 4) {
        return ZmErrRcvTo;
      }
      info.state = RStart;
      return SendRinit(&info);

    case RCrc:
    case RFile:
    case RData:
      ZStatus(SndTimeout, info.timeoutCount, NULL);
      if (info.timeoutCount > 2) {
        info.timeoutCount = 0;
        info.state = RStart;
        return SendRinit(&info);
      }
      return info.state == RCrc ? ResendCrcReq(&info): ResendRpos(&info);

    case RFinish:
      ZStatus(SndTimeout, info.timeoutCount, NULL);
      return ZmDone;

    case YRStart:
    case YRDataWait:
    case YRData:
    case YREOF:
      return YrcvTimeout(&info);

      /* transmit */
    case TStart:
      /* waiting for INIT frame from other end */
    case TInit:
      /* sent INIT, waiting for ZACK */
    case FileWait:
      /* sent file header, waiting for ZRPOS */
    case CrcWait:
      /* sent file crc, waiting for ZRPOS */
    case SendWait:
      /* waiting for ZACK */
    case SendEof:
      /* sent EOF, waiting for ZACK */
    case TFinish:
      /* sent ZFIN, waiting for ZFIN */
    case YTStart:
    case YTFile:
    case YTDataWait:
    case YTData:
    case YTEOF:
    case YTFin:
      ZStatus(RcvTimeout, 0, NULL);
      return ZmErrRcvTo;

    case Sending:
      /* sending data subpackets, ready for int */
      return SendMoreFileData(&info);

      /* general */
    case CommandData:
      /* waiting for command data */
    case StderrData:
      /* waiting for stderr data */
      return ZmErrSndTo;
    case CommandWait:
      /* waiting for command to execute */
      return ZmErrCmdTo;
    case Done:
      return ZmDone;
    default:
      return 0;
  }
}



int FQTermZmodem::ZmodemAttention(ZModem *info) {
  /* attention received from remote end */
  if (info->state == Sending) {
    ZOFlush(info);
    info->interrupt = 1;
  }
  return 0;
}

int FQTermZmodem::YmodemTInit(ZModem *info) {
  info->state = YTStart;
  info->Protocol = YMODEM;
  info->errCount = 0;
  info->InputState = Ysend;
  info->canCount = info->chrCount = 0;
  info->windowCount = 0;
  info->filename = NULL;

  if (info->packetsize != 1024) {
    info->packetsize = 128;
  }

  info->buffer = (uchar*)malloc(1024);

  ZIFlush(info);
  ZFlowControl(0, info);

  info->timeout = 60;

  return 0;
}


int FQTermZmodem::XmodemTInit(ZModem *info) {
  (void)YmodemTInit(info);
  info->Protocol = XMODEM;
  return 0;
}


int FQTermZmodem::YmodemRInit(ZModem *info) {
  info->errCount = 0;
  info->InputState = Yrcv;
  info->canCount = info->chrCount = 0;
  info->noiseCount = 0;
  info->filename = NULL;
  info->file = NULL;

  if (info->buffer == NULL) {
    info->buffer = (uchar*)malloc(1024);
  }

  info->state = YRStart;
  info->packetCount = -1;
  info->timeoutCount = 0;
  info->timeout = 10;
  info->offset = 0;

  ZIFlush(info);

  return ZXmitStr((uchar*)"C", 1, info);
}


int FQTermZmodem::XmodemRInit(ZModem *info) {
#if 0
  int err;

  state = Start;
  protocol = prot;
  ymodem = prot == Ymodem || prot == YmodemG;

  if (ymodem) {
    strcpy(xmDefPath, file);
  } else {
    strcpy(xmFilename, file);
  }

  eotCount = errorCount = errorCount2 = 0;

  if (err = XmodemRStart()) {
    return err;
  }

  state = Init;
  packetId = ymodem ? 255 : 0;
  packetCount = 0;

  pktHdrLen = protocol == Xmodem ? 3 : 4;
#endif
  return 0;
}


ulong FQTermZmodem::FileCrc(char *name) {
  ulong crc;
  FILE *ifile = fopen(name, "rb");
  int i;

  if (ifile == NULL)
    /* shouldn't happen, since we did access(2) */ {
    return 0;
  }

  crc = 0xffffffff;

  while ((i = fgetc(ifile)) != EOF) {
    crc = UPDC32(i, crc);
  }

  fclose(ifile);
  return ~crc;
}



const char *FQTermZmodem::sname(ZModem *info) {
  return sname2(info->state);
}

const char *FQTermZmodem::sname2(ZMState state) {
  const char *names[] =  {
    "RStart", "RSinitWait", "RFileName", "RCrc", "RFile", "RData", "RDataErr",
	"RFinish", "TStart", "TInit", "FileWait", "CrcWait", "Sending", "SendWait",
	"SendDone", "SendEof", "TFinish", "CommandData", "CommandWait", "StderrData",
	"Done", "YTStart", "YTFile", "YTDataWait", "YTData", "YTEOF", "YTFin",
	"YRStart", "YRDataWait", "YRData", "YREOF"
  };

  return names[(int)state];

}


int FQTermZmodem::ZXmitChr(uchar c, ZModem *info) {
  // to be completed
  return 0;
}

int FQTermZmodem::ZXmitStr(const uchar *str, int len, ZModem *info) {
  //to be completed
  telnet_->write((const char*)str, (uint)len);
  return 0;
}

void FQTermZmodem::ZIFlush(ZModem *info) {
  //to be completed
}

void FQTermZmodem::ZOFlush(ZModem *info) {
  //to be completed
}

int FQTermZmodem::ZAttn(ZModem *info) {
  //to be completed
  return 0;
}

void FQTermZmodem::ZStatus(int type, int value, const char *status) {
  emit ZmodemState(type, value, status);
  switch (type) {
    case RcvByteCount:
      FQ_TRACE("zmodem", 5) << value << " bytes received.";
      break;
    case SndByteCount:
      FQ_TRACE("zmodem", 5) << value << " bytes sent.";
      break;
    case RcvTimeout:
      /* receiver did not respond, aborting */
      FQ_TRACE("zmodem", 0) << "Zmodem time out!";
      break;
    case SndTimeout:
      /* value is # of consecutive send timeouts */
      FQ_TRACE("zmodem", 0) << "Zmodem time out after trying "
                            << value << " times";
      break;
    case RmtCancel:
      /* remote end has cancelled */
      FQ_TRACE("zmodem", 1) << "Zmodem canceled by remote peer";
      break;
    case ProtocolErr:
      /* protocol error has occurred, val=hdr */
      FQ_TRACE("zmodem", 0) << "Unhandled header " << value
                            << " at state " << status;
      break;
    case RemoteMessage:
      /* message from remote end */
      FQ_TRACE("zmodem", 0) << "Message from remote peer: " << status;
      break;
    case DataErr:
      /* data error, val=error count */
      FQ_TRACE("zmodem", 0) << "Data errors " << value;
      break;
    case FileErr:
      /* error writing file, val=errno */
      FQ_TRACE("zmodem", 0) << "Falied to write file.";
      break;
    case FileBegin:
      /* file transfer begins, str=name */
      FQ_TRACE("zmodem", 0) << "Starting file: " << status;
      break;
    case FileEnd:
      /* file transfer ends, str=name */
      FQ_TRACE("zmodem", 0) << "Finishing file: " << status;
      break;
    case FileSkip:
      /* file being skipped, str=name */
      FQ_TRACE("zmodem", 0) << "Skipping file: " << status;
      break;
  }
}

FILE *FQTermZmodem::ZOpenFile(char *name, ulong crc, ZModem *info) {
  //to be complete
  FILE *rval;
  int apnd = 0;
  QString strText;
  strText = encoding2unicode(name, serverEncodingID);
  QString str = FQTermPref::getInstance()->zmodemDir_ + strText;

  QString zpath = QFileInfo(FQTermPref::getInstance()->zmodemDir_).absoluteFilePath();
  QString path = QFileInfo(str).absoluteFilePath();

  lastDownloadedFilename_ = path;

  if (path.startsWith(zpath)) {
    // Ensure that file is only saved under zmodemDir_.
    // TODO: lazy, should use bbs2unicode
    rval = fopen(str.toLocal8Bit(), apnd ? "ab" : "wb");
  } else {
    rval = NULL;
  }

  if (rval == NULL) {
    perror(name);
  }

  return rval;
}

int FQTermZmodem::ZXmitHdr(int type, int format, const uchar data[4], ZModem *info) {
  if (format == ZBIN && info->crc32) {
    format = ZBIN32;
  }

  switch (format) {
    case ZHEX:
      return ZXmitHdrHex(type, data, info);

    case ZBIN:
      return ZXmitHdrBin(type, data, info);

    case ZBIN32:
      return ZXmitHdrBin32(type, data, info);

    default:
      return 0;
  }
}


int FQTermZmodem::ZXmitHdrHex(int type, const uchar data[4], ZModem *info) {
  uchar buffer[128];
  uchar *ptr = buffer;
  uint crc;
  int i;

  zmodemlog("sending  %s: %2.2x %2.2x %2.2x %2.2x = %lx\n", hdrnames[type],
            data[0], data[1], data[2], data[3], ZDec4(data));

  *ptr++ = ZPAD;
  *ptr++ = ZPAD;
  *ptr++ = ZDLE;
  *ptr++ = ZHEX;

  ptr = putHex(ptr, type);
  crc = updcrc(type, 0);
  for (i = 4; --i >= 0; ++data) {
    ptr = putHex(ptr,  *data);
    crc = updcrc(*data, crc);
  }
  crc = updcrc(0, crc);
  crc = updcrc(0, crc);
  ptr = putHex(ptr, (crc >> 8) &0xff);
  ptr = putHex(ptr, crc &0xff);
  *ptr++ = '\r';
  *ptr++ = '\n';
  if (type != ZACK && type != ZFIN) {
    *ptr++ = XON;
  }

  return ZXmitStr(buffer, ptr - buffer, info);
}


int FQTermZmodem::ZXmitHdrBin(int type, const uchar data[4], ZModem *info) {
  uchar buffer[128];
  uchar *ptr = buffer;
  uint crc;
  int len;

  zmodemlog("sending  %s: %2.2x %2.2x %2.2x %2.2x = %lx\n", hdrnames[type],
            data[0], data[1], data[2], data[3], ZDec4(data));

  *ptr++ = ZPAD;
  *ptr++ = ZDLE;
  *ptr++ = ZBIN;

  ptr = putZdle(ptr, type, info);
  crc = updcrc(type, 0);
  for (len = 4; --len >= 0; ++data) {
    ptr = putZdle(ptr,  *data, info);
    crc = updcrc(*data, crc);
  }
  crc = updcrc(0, crc);
  crc = updcrc(0, crc);
  ptr = putZdle(ptr, (crc >> 8) &0xff, info);
  ptr = putZdle(ptr, crc &0xff, info);

  len = ptr - buffer;
  return ZXmitStr(buffer, len, info);
}

int FQTermZmodem::ZXmitHdrBin32(int type, const uchar data[4], ZModem *info) {
  uchar buffer[128];
  uchar *ptr = buffer;
  ulong crc;
  int len;

  zmodemlog("sending  %s: %2.2x %2.2x %2.2x %2.2x = %lx\n", hdrnames[type],
            data[0], data[1], data[2], data[3], ZDec4(data));

  *ptr++ = ZPAD;
  *ptr++ = ZDLE;
  *ptr++ = ZBIN32;
  ptr = putZdle(ptr, type, info);
  crc = UPDC32(type, 0xffffffffL);
  for (len = 4; --len >= 0; ++data) {
    ptr = putZdle(ptr,  *data, info);
    crc = UPDC32(*data, crc);
  }
  crc = ~crc;
  for (len = 4; --len >= 0; crc >>= 8) {
    ptr = putZdle(ptr, crc &0xff, info);
  }

  len = ptr - buffer;
  return ZXmitStr(buffer, len, info);
}


uchar *FQTermZmodem::putZdle(uchar *ptr, uchar c, ZModem *info) {
  uchar c2 = c &0177;

  if (c == ZDLE || c2 == 020 || c2 == 021 || c2 == 023 || c2 == 0177 || (c2 ==
                                                                         015 && connectionType == 0 /*&& info->atSign*/) ||
#ifdef COMMENT
      c2 == 035 || (c2 == '~' && info->lastCR) ||
#endif /* COMMENT */
      c2 == 035 || (c2 < 040 && info->escCtrl)) {
    *ptr++ = ZDLE;
    if (c == 0177) {
      *ptr = ZRUB0;
    } else if (c == 0377) {
      *ptr = ZRUB1;
    } else {
      *ptr = c ^ 0100;
    }
  } else {
    *ptr = c;
  }

  info->atSign = c2 == '@';
  info->lastCR = c2 == '\r';

  return ++ptr;
}


uchar *FQTermZmodem::ZEnc4(ulong n) {
  static uchar buf[4];
  buf[0] = n &0xff;
  n >>= 8;
  buf[1] = n &0xff;
  n >>= 8;
  buf[2] = n &0xff;
  n >>= 8;
  buf[3] = n &0xff;
  return buf;
}

ulong FQTermZmodem::ZDec4(const uchar buf[4]) {
  return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}




int FQTermZmodem::YrcvChar(char c, ZModem *info) {
  int err;

  if (info->canCount >= 2) {
    ZStatus(RmtCancel, 0, NULL);
    return ZmErrCancel;
  }

  switch (info->state) {
    case YREOF:
      if (c == EOT) {
        ZCloseFile(info);
        info->file = NULL;
        ZStatus(FileEnd, 0, info->filename);
        if (info->filename != NULL) {
          free(info->filename);
        }
        if ((err = acceptPacket(info)) != 0) {
          return err;
        }
        info->packetCount = -1;
        info->offset = 0;
        info->state = YRStart;
        return ZXmitStr((uchar*)"C", 1, info);
      }
      /* else, drop through */

    case YRStart:
    case YRDataWait:
      switch (c) {
        case SOH:
        case STX:
          info->pktLen = c == SOH ? (128+4): (1024+4);
          info->state = YRData;
          info->chrCount = 0;
          info->timeout = 1;
          info->noiseCount = 0;
          info->crc = 0;
          break;

        case EOT:
          /* ignore first EOT to protect against false eot */
          info->state = YREOF;
          return rejectPacket(info);

        default:
          if (++info->noiseCount > 135) {
            return ZXmitStr(NakStr, 1, info);
          }
          break;
      }
      break;

    case YRData:
      info->buffer[info->chrCount++] = c;
      if (info->chrCount >= info->pktLen) {
        return ProcessPacket(info);
      }
      break;

    default:
      break;
  }

  return 0;
}

int FQTermZmodem::YrcvTimeout(ZModem *info) {
  switch (info->state) {
    case YRStart:
      if (info->timeoutCount >= 10) {
        (void)ZXmitStr(CanStr, 2, info);
        return ZmErrInitTo;
      }
      return ZXmitStr((uchar*)"C", 1, info);

    case YRDataWait:
    case YREOF:
    case YRData:
      if (info->timeoutCount >= 10) {
        (void)ZXmitStr(CanStr, 2, info);
        return ZmErrRcvTo;
      }
      return ZXmitStr(NakStr, 1, info);
    default:
      return 0;
  }
}


void FQTermZmodem::ZIdleStr(uchar *buffer, int len, ZModem *info) {
  //to be completed
}


int FQTermZmodem::FinishChar(char c, ZModem *info) {
  if (c == 'O') {
    if (++info->chrCount >= 2) {
      return ZmDone;
    }
  } else {
    info->chrCount = 0;
  }
  return 0;
}



int FQTermZmodem::DataChar(uchar c, ZModem *info) {
  if (c == ZDLE) {
    info->escape = 1;
    return 0;
  }

  if (info->escape) {
    info->escape = 0;
    switch (c) {
      case ZCRCE:
      case ZCRCG:
      case ZCRCQ:
      case ZCRCW:
        info->PacketType = c;
        info->crcCount = (info->DataType == ZBIN32) ? 4 : 2;
        if (info->DataType == ZBIN) {
          info->crc = updcrc(c, info->crc);
        } else {
          info->crc = UPDC32(c, info->crc);
        }
        return 0;
      case ZRUB0:
        c = 0177;
        break;
      case ZRUB1:
        c = 0377;
        break;
      default:
        c ^= 0100;
        break;
    }
  }
  if (connectionType == 0) {
    if (lastPullByte == 0x0d && c == 0x00) {
      lastPullByte = 0;
      return 0;
    } else if (lastPullByte == 0xff && c == 0xff) {
      lastPullByte = 0;
      return 0;
    }
  }
  lastPullByte = c;

  switch (info->DataType) {
    /* TODO: are hex data packets ever used? */
    case ZBIN:
      info->crc = updcrc(c, info->crc);
      if (info->crcCount == 0) {
        info->buffer[info->chrCount++] = c;
      } else if (--info->crcCount == 0) {
        return ZDataReceived(info, (info->crc &0xffff) == 0);
      }
      break;


    case ZBIN32:
      info->crc = UPDC32(c, info->crc);
      if (info->crcCount == 0) {
        info->buffer[info->chrCount++] = c;
      } else if (--info->crcCount == 0) {
        return ZDataReceived(info, info->crc == 0xdebb20e3);
      }
      break;
  }
  return 0;
}


int FQTermZmodem::HdrChar(uchar c, ZModem *info) {
  int i;
  int crc = 0;

  if (c == ZDLE) {
    info->escape = 1;
    return 0;
  }

  if (info->escape) {
    info->escape = 0;
    switch (c) {
      case ZRUB0:
        c = 0177;
        break;
      case ZRUB1:
        c = 0377;
        break;
      default:
        c ^= 0100;
        break;
    }
  }

  if (info->chrCount == 0) {
    /* waiting for format */
    switch (c) {
      case ZHEX:
      case ZBIN:
      case ZBIN32:
        info->DataType = c;
        info->chrCount = 1;
        info->crc = (info->DataType != ZBIN32) ? 0 : 0xffffffffL;
        memset(info->hdrData, 0, sizeof(info->hdrData));
        break;
      default:
        info->InputState = Idle;
        info->chrCount = 0;
        return ZXmitHdrHex(ZNAK, zeros, info);
    }
    return 0;
  }


  switch (info->DataType) {
    /* hex header is 14 hex digits, cr, lf.  Optional xon is ignored */
    case ZHEX:
      if (info->chrCount <= 14 && !isxdigit(c)) {
        info->InputState = Idle;
        info->chrCount = 0;
        return ZXmitHdrHex(ZNAK, zeros, info);
      }

      if (info->chrCount <= 14) {
        i = (info->chrCount - 1) / 2;
        info->hdrData[i] = rcvHex(info->hdrData[i], c);
      }

      if (info->chrCount == 16) {
        crc = 0;
        for (i = 0; i < 7; ++i) {
          crc = updcrc(info->hdrData[i], crc);
        }
        info->InputState = Idle;
        info->chrCount = 0;
        if ((crc &0xffff) != 0) {
          return ZXmitHdrHex(ZNAK, zeros, info);
        } else {
          return ZProtocol(info);
        }
      }
      else {
        ++info->chrCount;
      }
      break;


    case ZBIN:
      /* binary header is type, 4 bytes data, 2 bytes CRC */
      info->hdrData[info->chrCount - 1] = c;
      info->crc = updcrc(c, info->crc);
      if (++info->chrCount > 7) {
        info->InputState = Idle;
        info->chrCount = 0;
        if ((crc &0xffff) != 0) {
          return ZXmitHdrHex(ZNAK, zeros, info);
        } else {
          return ZProtocol(info);
        }
      }
      break;


    case ZBIN32:
      /* binary32 header is type, 4 bytes data, 4 bytes CRC */
      info->hdrData[info->chrCount - 1] = c;
      info->crc = UPDC32(c, info->crc);
      if (++info->chrCount > 9) {
        info->InputState = Idle;
        info->chrCount = 0;
        if (info->crc != 0xdebb20e3)
          /* see note below */ {
          return ZXmitHdrHex(ZNAK, zeros, info);
        } else {
          return ZProtocol(info);
        }
      }
      break;
  }
  return 0;
}




int FQTermZmodem::IdleChar(uchar c, ZModem *info) {
  if (info->chrCount == 0) {
    if (c == ZPAD) {
      ++info->chrCount;
    } else if (info->state == Sending && ++info->noiseCount > MaxNoise) {
      info->waitflag = 1;
    } else if (info->state == TStart && (c == 'C' || c == 'G' || c == NAK)) {
      /* switch to ymodem */
      info->state = YTStart;
      info->InputState = Ysend;
      info->Protocol = YMODEM;
      return YsendChar(c, info);
    } else {
      ZIdleStr(&c, 1, info);
    }
  }

  else {
    switch (c) {
      case ZPAD:
        ++info->chrCount;
        break;
      case ZDLE:
        info->InputState = Inhdr;
        info->chrCount = 0;
        break;
      default:
        while (--info->chrCount >= 0) {
          ZIdleStr((uchar*)"*", 1, info);
        }
        info->chrCount = 0;
        break;
    }
  }
  return 0;
}



int FQTermZmodem::YsendChar(char c, ZModem *info) {
  int err;

  if (info->canCount >= 2) {
    ZStatus(RmtCancel, 0, NULL);
    return ZmErrCancel;
  }

  switch (info->state) {
    case YTStart:
      /* wait for 'G', 'C' or NAK */
      switch (c) {
        case 'G':
          /* streaming YModem */
        case 'C':
          /* CRC YModem */
        case NAK:
          /* checksum YModem */
          info->PacketType = c;
          return ZmDone;
        default:
          return 0;
      }

    case YTFile:
      /* sent filename, waiting for ACK or NAK */
      switch (c) {
        case NAK:
          /* resend */
        case 'C':
        case 'G':
          ZStatus(DataErr, ++info->errCount, NULL);
          return YSendFilename(info);
        case ACK:
          info->state = YTDataWait;
        default:
          return 0;
      }

    case YTDataWait:
      /* sent filename, waiting for G,C or NAK */
      switch (c) {
        case NAK:
        case 'C':
        case 'G':
          info->chrCount = 0;
          if (info->PacketType == 'G') {
            /* send it all at once */
            while (info->state == YTData) {
              if ((err = YSendData(info))) {
                return err;
              }
            }
            return 0;
          } else {
            return YSendData(info);
          }
          break;
        default:
          return 0;
      }

    case YTData:
      /* sent data, waiting for ACK or NAK */
      switch (c) {
        case 'C':
        case 'G':
          /* protocol failure, resend filename */
          if (info->Protocol == YMODEM) {
            ZStatus(DataErr, ++info->errCount, NULL);
            info->state = YTFile;
            rewind(info->file);
            return YSendFilename(info);
          }
          /* else XModem, treat it like a NAK */
        case NAK:
          ZStatus(DataErr, ++info->errCount, NULL);
          return YXmitData(info->buffer + info->bufp, info->ylen, info);
        case ACK:
          info->offset += info->ylen;
          info->bufp += info->ylen;
          info->chrCount -= info->ylen;
          ZStatus(SndByteCount, info->offset, NULL);
          return YSendData(info);
        default:
          return 0;
      }

    case YTEOF:
      /* sent EOF, waiting for ACK or NAK */
      switch (c) {
        case NAK:
          return ZXmitStr(eotstr, 1, info);
        case ACK:
          info->state = info->Protocol == YMODEM ? YTStart : Done;
          return ZmDone;
        default:
          return 0;
      }

    case YTFin:
      /* sent Fin, waiting for ACK or NAK */
      switch (c) {
        case NAK:
          return YSendFin(info);
        case ACK:
          return ZmDone;
        default:
          return 0;
      }
    default:
      return 0;
  }
}



int FQTermZmodem::ZProtocol(ZModem *info) {
  StateTable *table;

  zmodemlog("received %s: %2.2x %2.2x %2.2x %2.2x = %lx\n",
            hdrnames[info->hdrData[0]], info->hdrData[1], info->hdrData[2],
            info->hdrData[3], info->hdrData[4], ZDec4(info->hdrData + 1));

  /* Flags are sent in F3 F2 F1 F0 order.  Data is sent in P0 P1 P2 P3 */

  info->timeoutCount = 0;
  info->noiseCount = 0;

  //	zmodemTimer->start(info->timeout*1000);

  table = tables[(int)info->state];
  while (table->type != 99 && table->type != info->hdrData[0]) {
    ++table;
  }

  zmodemlog("  state %s => %s, iflush=%d, oflush=%d, call %x\n", sname(info),
            sname2(table->newstate), table->IFlush, table->OFlush, table->func);

  info->state = table->newstate;

  if (table->IFlush) {
    info->rcvlen = 0;
    ZIFlush(info);
  }
  if (table->OFlush) {
    ZOFlush(info);
  }
  return (this->*(table->func))(info);
}


int FQTermZmodem::ZDataReceived(ZModem *info, int crcGood) {
  switch (info->state) {
    case RSinitWait:
      return GotSinitData(info, crcGood);
    case RFileName:
      return GotFileName(info, crcGood);
    case RData:
      return GotFileData(info, crcGood);
    case CommandData:
      return GotCommandData(info, crcGood);
    case StderrData:
      return GotStderrData(info, crcGood);
    default:
      return ZPF(info);
  }
}


int FQTermZmodem::ZPF(ZModem *info) {
  info->waitflag = 1; /* pause any in-progress transmission */
  ZStatus(ProtocolErr, info->hdrData[0], sname(info));
  return 0;
}


int FQTermZmodem::Ignore(ZModem *info) {
  return 0;
}


int FQTermZmodem::AnswerChallenge(ZModem *info) {
  return ZXmitHdrHex(ZACK, info->hdrData + 1, info);
}


int FQTermZmodem::GotAbort(ZModem *info) {
  ZStatus(RmtCancel, 0, NULL);
  return ZXmitHdrHex(ZFIN, zeros, info);
}


int FQTermZmodem::GotCancel(ZModem *info) {
  return ZmErrCancel;
}


int FQTermZmodem::GotCommand(ZModem *info) {
  uchar rbuf[4];
  /* TODO: add command capability */


  /// EPERM not defined????????

  rbuf[0] = EPERM;
  rbuf[1] = rbuf[2] = rbuf[3] = 0;
  return ZXmitHdrHex(ZCOMPL, rbuf, info);
}

int FQTermZmodem::GotStderr(ZModem *info) {
  info->InputState = Indata;
  info->chrCount = 0;
  return 0;
}


int FQTermZmodem::RetDone(ZModem *info) {


  return ZmDone;
}


int FQTermZmodem::GotCommandData(ZModem *info, int crcGood) {
  /* TODO */
  return 0;
}


int FQTermZmodem::GotStderrData(ZModem *info, int crcGood) {
  info->buffer[info->chrCount] = '\0';
  ZStatus(RemoteMessage, info->chrCount, (char*)info->buffer);
  return 0;
}


int FQTermZmodem::GotFileName(ZModem *info, int crcGood) {
  info->InputState = Idle;
  info->chrCount = 0;

  if (!crcGood) {
    zmodemlog("GotFileName[%s]: bad crc, send ZNAK\n", sname(info));
    info->state = RStart;
    return ZXmitHdrHex(ZNAK, zeros, info);
  }

  parseFileName(info, (char*)info->buffer);

  if ((info->f1 &ZMMASK) == ZMCRC) {
    info->state = RCrc;
    return ZXmitHdrHex(ZCRC, zeros, info);
  }

  zmodemlog("GotFileName[%s]: good crc, call requestFile\n", sname(info));
  info->state = RFile;
  return requestFile(info, 0);
}


int FQTermZmodem::ResendCrcReq(ZModem *info) {
  zmodemlog("ResendCrcReq[%s]: send ZCRC\n", sname(info));
  return ZXmitHdrHex(ZCRC, zeros, info);
}


int FQTermZmodem::GotSinitData(ZModem *info, int crcGood) {
  info->InputState = Idle;
  info->chrCount = 0;
  info->state = RStart;

  zmodemlog("GotSinitData[%s]: crcGood=%d\n", sname(info), crcGood);

  if (!crcGood) {
    return ZXmitHdrHex(ZNAK, zeros, info);
  }

  if (info->attn != NULL) {
    free(info->attn);
  }
  info->attn = NULL;
  if (info->buffer[0] != '\0') {
    info->attn = strdup((char*)info->buffer);
  }
  return ZXmitHdrHex(ZACK, ZEnc4(SerialNo), info);
}


int FQTermZmodem::ResendRpos(ZModem *info) {
  zmodemlog("ResendRpos[%s]: send ZRPOS(%ld)\n", sname(info), info->offset);
  return ZXmitHdrHex(ZRPOS, ZEnc4(info->offset), info);
}


int FQTermZmodem::GotFileData(ZModem *info, int crcGood) {
  /* OK, now what?  Fushing the buffers and executing the
   * attn sequence has likely chopped off the input stream
   * mid-packet.  Now we switch to idle mode and treat all
   * incoming stuff like noise until we get a new valid
   * packet.
   */

  if (!crcGood) {
    /* oh bugger, an error. */
    zmodemlog("GotFileData[%s]: bad crc, send ZRPOS(%ld), new state = RFile\n",
              sname(info), info->offset);
    ZStatus(DataErr, ++info->errCount, NULL);
    if (info->errCount > MaxErrs) {
      ZmodemAbort(info);
      return ZmDataErr;
    } else {
      info->state = RFile;
      info->InputState = Idle;
      info->chrCount = 0;
      return fileError(info, ZRPOS, info->offset);
    }
  }

  if (ZWriteFile(info->buffer, info->chrCount, info->file, info)) {
    /* RED ALERT!  Could not write the file. */
    ZStatus(FileErr, zerrno, NULL);
    info->state = RFinish;
    info->InputState = Idle;
    info->chrCount = 0;
    return fileError(info, ZFERR, zerrno);
  }

  zmodemlog("GotFileData[%s]: %ld.%d,", sname(info), info->offset,
            info->chrCount);
  info->offset += info->chrCount;
  ZStatus(RcvByteCount, info->offset, NULL);

  /* if this was the last data subpacket, leave data mode */
  if (info->PacketType == ZCRCE || info->PacketType == ZCRCW) {
    zmodemlog("  ZCRCE|ZCRCW, new state RFile");
    info->state = RFile;
    info->InputState = Idle;
    info->chrCount = 0;
  } else {
    zmodemlog("  call dataSetup");
    (void)dataSetup(info);
  }

  if (info->PacketType == ZCRCQ || info->PacketType == ZCRCW) {
    zmodemlog(",  send ZACK\n");
    return ZXmitHdrHex(ZACK, ZEnc4(info->offset), info);
  } else {
    zmodemlog("\n");
  }

  return 0;
}

int FQTermZmodem::SendRinit(ZModem *info) {
  uchar dbuf[4];



#ifdef COMMENT
  if (info->timeoutCount >= 5)
	/* TODO: switch to Ymodem */
#endif /* COMMENT */
  {
    transferstate = transferstart;
  }
  //transfer would be active, it must be set to false when transfer complete or abort
  zmodemlog("SendRinit[%s]: send ZRINIT\n", sname(info));

  info->timeout = ResponseTime;
  dbuf[0] = info->bufsize &0xff;
  dbuf[1] = (info->bufsize >> 8) &0xff;
  dbuf[2] = 0;
  dbuf[3] = info->zrinitflags;
  return ZXmitHdrHex(ZRINIT, dbuf, info);
}


int FQTermZmodem::SendMoreFileData(ZModem *info) {
  int type;
  int qfull = 0;
  int err;
  int len; /* max # chars to send this packet */
  long pending; /* # of characters sent but not acknowledged */

  /* ZCRCE: CRC next, frame ends, header follows
   * ZCRCG: CRC next, frame continues nonstop
   * ZCRCQ: CRC next, send ZACK, frame continues nonstop
   * ZCRCW: CRC next, send ZACK, frame ends, header follows
   */

  if (info->interrupt) {
    /* Bugger, receiver sent an interrupt.  Enter a wait state
     * and see what they want.  Next header *should* be ZRPOS.
     */
    info->state = SendWait;
    info->timeout = 60;
    return 0;
  }

  /* Find out how many bytes we can transfer in the next packet */

  len = info->packetsize;

  pending = info->offset - info->lastOffset;

  if (info->windowsize != 0 && info->windowsize - pending <= len) {
    len = info->windowsize - pending;
    qfull = 1;
  }
  if (info->bufsize != 0 && info->bufsize - pending <= len) {
    len = info->bufsize - pending;
    qfull = 1;
  }

  if (len == 0) {
    /* window still full, keep waiting */
    info->state = SendWait;
    info->timeout = 60;
    return 0;
  }


  /* OK, we can safely transmit 'len' bytes of data.  Start reading
   * file until buffer is full.
   */

  len -= 10; /* Pre-deduct 10 bytes for trailing CRC */


  /* find out what kind of packet to send */
  if (info->waitflag) {
    type = ZCRCW;
    info->waitflag = 0;
  }
#ifdef COMMENT
  else if (info->fileEof) {
    type = ZCRCE;
  }
#endif /* COMMENT */
  else if (qfull) {
    type = ZCRCW;
  } else {
	switch (info->Streaming) {
      case Full:
      case Segmented:
        type = ZCRCG;
        break;

      case StrWindow:
        if ((info->windowCount += len) < info->windowsize / 4) {
          type = ZCRCG;
        } else {
          type = ZCRCQ;
          info->windowCount = 0;
        }
        break;

      default:
      case SlidingWindow:
        type = ZCRCQ;
        break;
	}
  }

  {
    int crc32 = info->crc32;
    int c = 0, c2, atSign = 0;
    ulong crc;
    uchar *ptr = info->buffer;

    crc = crc32 ? 0xffffffff : 0;

    /* read characters from file and put into buffer until buffer is
     * full or file is exhausted
     */


    while (len > 0 && (c = getc(info->file)) != EOF) {
      if (!crc32) {
        crc = updcrc(c, crc);
      } else {
        crc = UPDC32(c, crc);
      }

      /* zmodem protocol requires that CAN(ZDLE), DLE, XON, XOFF and
       * a CR following '@' be escaped.  In addition, I escape '^]'
       * to protect telnet, "<CR>~." to protect rlogin, and ESC for good
       * measure.
       */
      c2 = c &0177;
      if (c == ZDLE || c2 == 020 || c2 == 021 || c2 == 023 || c2 == 0177 || c2 ==
          '\r' || c2 == '\n' || c2 == 033 || c2 == 035 || (c2 < 040 && info->escCtrl)) {
        *ptr++ = ZDLE;
        if (c == 0177) {
          *ptr = ZRUB0;
        } else if (c == 0377) {
          *ptr = ZRUB1;
        } else {
          *ptr = c ^ 0100;
        }
        len -= 2;
      } else {
        *ptr = c;
        --len;
      }
      ++ptr;

      atSign = c2 == '@';
      ++info->offset;
    }

    /* if we've reached file end, a ZEOF header will follow.  If
     * there's room in the outgoing buffer for it, end the packet
     * with ZCRCE and append the ZEOF header.  If there isn't room,
     * we'll have to do a ZCRCW
     */
    if ((info->fileEof = (c == EOF))) {
      if (qfull || (info->bufsize != 0 && len < 24)) {
        type = ZCRCW;
      } else {
        type = ZCRCE;
      }
    }

    *ptr++ = ZDLE;
    if (!crc32) {
      crc = updcrc(type, crc);
    } else {
      crc = UPDC32(type, crc);
    }
    *ptr++ = type;

    if (!crc32) {
      crc = updcrc(0, crc);
      crc = updcrc(0, crc);
      ptr = putZdle(ptr, (crc >> 8) &0xff, info);
      ptr = putZdle(ptr, crc &0xff, info);
    } else {
      crc = ~crc;
      for (len = 4; --len >= 0; crc >>= 8) {
        ptr = putZdle(ptr, crc &0xff, info);
      }
    }

    len = ptr - info->buffer;
  }

  ZStatus(SndByteCount, info->offset, NULL);

  if ((err = ZXmitStr(info->buffer, len, info))) {
    return err;
  }

#ifdef COMMENT
  if ((err = ZXmitData(ZBIN, len, uchar(type), info->buffer, info))) {
    return err;
  }
#endif /* COMMENT */

  /* finally, do we want to wait after this packet? */

  switch (type) {
    case ZCRCE:
      info->state = SendEof;
      info->timeout = 60;
      return ZXmitHdrHex(ZEOF, ZEnc4(info->offset), info);
    case ZCRCW:
      info->state = info->fileEof ? SendDone : SendWait;
      info->timeout = 60;
      break;
    default:
      info->state = Sending;
      info->timeout = 0;
      break;
  }


#ifdef COMMENT
  if (info->fileEof) {
    /* Yes, file is done, send EOF and wait */
    info->state = SendEof;
    info->timeout = 60;
    return ZXmitHdrHex(ZEOF, ZEnc4(info->offset), info);
  } else if (type == ZCRCW) {
    info->state = SendWait;
    info->timeout = 60;
  } else {
    info->state = Sending;
    info->timeout = 0;
  }
#endif /* COMMENT */
  return 0;
}


uint FQTermZmodem::rcvHex(uint i, char c) {
  if (c <= '9') {
    c -= '0';
  } else if (c <= 'F') {
    c -= 'A' - 10;
  } else {
    c -= 'a' - 10;
  }
  return (i << 4) + c;
}


int FQTermZmodem::dataSetup(ZModem *info) {
  info->InputState = Indata;
  info->chrCount = 0;
  info->crcCount = 0;
  info->crc = (info->DataType != ZBIN32) ? 0 : 0xffffffffL;
  return 0;
}

int FQTermZmodem::ZWriteFile(uchar *buffer, int len, FILE *file, ZModem*) {
  return (int)fwrite(buffer, 1, len, file) == len ? 0 : ZmErrSys;
}

int FQTermZmodem::ZCloseFile(ZModem *info) {
  //to be completed
  fclose(info->file);
  return 0;
}

void FQTermZmodem::ZFlowControl(int onoff, ZModem *info) {
  //to be completed
}

int FQTermZmodem::GotSinit(ZModem *info) {
  zmodemlog("GotSinit[%s]: call dataSetup\n", sname(info));

  info->zsinitflags = info->hdrData[4];
  info->escCtrl = info->zsinitflags &TESCCTL;
  info->escHibit = info->zsinitflags &TESC8;
  ZFlowControl(1, info);
  return dataSetup(info);
}

int FQTermZmodem::GotFile(ZModem *info) {
  zmodemlog("GotFile[%s]: call dataSetup\n", sname(info));

  info->errCount = 0;
  info->f0 = info->hdrData[4];
  info->f1 = info->hdrData[3];
  info->f2 = info->hdrData[2];
  info->f3 = info->hdrData[1];
  return dataSetup(info);
}

int FQTermZmodem::GotFin(ZModem *info) {
  int i;
  zmodemlog("GotFin[%s]: send ZFIN\n", sname(info));
  info->InputState = Finish;
  info->chrCount = 0;
  if (info->filename != NULL) {
    free(info->filename);
  }
  i = ZXmitHdrHex(ZFIN, zeros, info);
  ZmodemReset(info);
  transferstate = transferstop; // transfer complete
  return i;
}


int FQTermZmodem::GotData(ZModem *info) {
  int err;

  zmodemlog("GotData[%s]:\n", sname(info));

  if (ZDec4(info->hdrData + 1) != info->offset) {
    if (info->attn != NULL && (err = ZAttn(info)) != 0) {
      return err;
    }
    zmodemlog("  bad, send ZRPOS(%ld)\n", info->offset);
    return ZXmitHdrHex(ZRPOS, ZEnc4(info->offset), info);
  }

  /* Let's do it! */
  zmodemlog("  call dataSetup\n");
  return dataSetup(info);
}

int FQTermZmodem::GotEof(ZModem *info) {
  zmodemlog("GotEof[%s]: offset=%ld\n", sname(info), info->offset);
  if (ZDec4(info->hdrData + 1) != info->offset) {
    zmodemlog("zdec4(info->hdrdata+1)=%ld\n", ZDec4(info->hdrData + 1));
    zmodemlog("  bad length, state = RFile\n");
    info->state = RFile;
    return 0; /* it was probably spurious */
  }

  /* TODO: if we can't close the file, send a ZFERR */

  ZCloseFile(info);
  info->file = NULL;
  ZStatus(FileEnd, 0, info->filename);
  if (info->filename != NULL) {
    free(info->filename);
    info->filename = NULL;
  }
  return SendRinit(info);
}

int FQTermZmodem::GotFreecnt(ZModem *info) {
  /* TODO: how do we find free space on system? */
  return ZXmitHdrHex(ZACK, ZEnc4(0xffffffff), info);
}


int FQTermZmodem::GotFileCrc(ZModem *info) {
  zmodemlog("GotFileCrc[%s]: call requestFile\n", sname(info));
  return requestFile(info, ZDec4(info->hdrData + 1));
}

int FQTermZmodem::requestFile(ZModem *info, ulong crc) {
  info->file = ZOpenFile((char*)info->buffer, crc, info);

  if (info->file == NULL) {
    zmodemlog("requestFile[%s]: send ZSKIP\n", sname(info));

    info->state = RStart;
    ZStatus(FileSkip, 0, info->filename);
    return ZXmitHdrHex(ZSKIP, zeros, info);
  } else {
    zmodemlog("requestFile[%s]: send ZRPOS(%ld)\n", sname(info), info->offset);
    info->offset = info->f0 == ZCRESUM ? ftell(info->file): 0;
    info->state = RFile;
    ZStatus(FileBegin, info->len, info->filename);
    return ZXmitHdrHex(ZRPOS, ZEnc4(info->offset), info);
  }
}

void FQTermZmodem::parseFileName(ZModem *info, char *fileinfo) {
  char *ptr;
  int serial = 0;

  info->len = info->mode = info->filesRem = info->bytesRem = info->fileType = 0;
  ptr = fileinfo + strlen(fileinfo) + 1;
  if (info->filename != NULL) {
    free(info->filename);
  }
  info->filename = strdup(fileinfo);
  sscanf(ptr, "%d %lo %o %o %d %d %d", &info->len, &info->date,  &info->mode,
         &serial, &info->filesRem, &info->bytesRem,  &info->fileType);
}



int FQTermZmodem::fileError(ZModem *info, int type, int data) {
  int err;

  info->InputState = Idle;
  info->chrCount = 0;

  if (info->attn != NULL && (err = ZAttn(info)) != 0) {
    return err;
  }
  return ZXmitHdrHex(type, ZEnc4(data), info);
}


int FQTermZmodem::ProcessPacket(ZModem *info) {
  int idx = (uchar)info->buffer[0];
  int idxc = (uchar)info->buffer[1];
  int crc0, crc1;
  int err;

  info->state = YRDataWait;

  if (idxc != 255-idx) {
    ZStatus(DataErr, ++info->errCount, NULL);
    return rejectPacket(info);
  }

  if (idx == (info->packetCount % 256))
    /* quietly ignore dup */ {
    return acceptPacket(info);
  }

  if (idx != (info->packetCount + 1) % 256) {
    /* out of sequence */
    (void)ZXmitStr(CanStr, 2, info);
    return ZmErrSequence;
  }

  crc0 = (uchar)info->buffer[info->pktLen - 2] << 8
         | (uchar)info->buffer[info->pktLen - 1];
  crc1 = calcCrc(info->buffer + 2, info->pktLen - 4);
  if (crc0 != crc1) {
    ZStatus(DataErr, ++info->errCount, NULL);
    return rejectPacket(info);
  }

  ++info->packetCount;

  if (info->packetCount == 0) {
    /* packet 0 is filename */
    if (info->buffer[2] == '\0') {
      /* null filename is FIN */
      (void)acceptPacket(info);
      return ZmDone;
    }

    parseFileName(info, (char*)info->buffer + 2);
    info->file = ZOpenFile(info->filename, 0, info);
    if (info->file == NULL) {
      (void)ZXmitStr(CanStr, 2, info);
      return ZmErrCantOpen;
    }
    if ((err = acceptPacket(info)) != 0) {
      return err;
    }
    return ZXmitStr((uchar*)"C", 1, info);
  }


  if (ZWriteFile(info->buffer + 2, info->pktLen - 4, info->file, info)) {
    ZStatus(FileErr, zerrno, NULL);
    (void)ZXmitStr(CanStr, 2, info);
    return ZmErrSys;
  }
  info->offset += info->pktLen - 4;
  ZStatus(RcvByteCount, info->offset, NULL);

  (void)acceptPacket(info);
  return 0;
}

int FQTermZmodem::rejectPacket(ZModem *info) {
  info->timeout = 10;
  return ZXmitStr(NakStr, 1, info);
}


int FQTermZmodem::acceptPacket(ZModem *info) {
  info->state = YRDataWait;
  info->timeout = 10;
  return ZXmitStr(AckStr, 1, info);
}


int FQTermZmodem::calcCrc(uchar *str, int len) {
  int crc = 0;
  while (--len >= 0) {
    crc = updcrc(*str++, crc);
  }
  crc = updcrc(0, crc);
  crc = updcrc(0, crc);
  return crc &0xffff;
}


char *FQTermZmodem::strdup(const char *str) {
  char *rval;
  int len = strlen(str) + 1;
  rval = (char*)malloc(len);
  strcpy(rval, str);
  return rval;
}


int FQTermZmodem::ZXmitData(int format, int len, uchar term, uchar *data, ZModem
                            *info) {
  uchar *ptr = info->buffer;
  uint crc;

  if (format == ZBIN && info->crc32) {
    format = ZBIN32;
  }

  zmodemlog("ZXmiteData: fmt=%c, len=%d, term=%c\n", format, len, term);

  crc = (format == ZBIN) ? 0 : 0xffffffff;

  while (--len >= 0) {
    if (format == ZBIN) {
      crc = updcrc(*data, crc);
    } else {
      crc = UPDC32(*data, crc);
    }
    ptr = putZdle(ptr,  *data++, info);
  }

  *ptr++ = ZDLE;
  if (format == ZBIN) {
    crc = updcrc(term, crc);
  } else {
    crc = UPDC32(term, crc);
  }
  *ptr++ = term;
  if (format == ZBIN) {
    crc = updcrc(0, crc);
    crc = updcrc(0, crc);
    ptr = putZdle(ptr, (crc >> 8) &0xff, info);
    ptr = putZdle(ptr, crc &0xff, info);
  } else {
    crc = ~crc;
    for (len = 4; --len >= 0; crc >>= 8) {
      ptr = putZdle(ptr, crc &0xff, info);
    }
  }

  return ZXmitStr(info->buffer, ptr - info->buffer, info);
}



int FQTermZmodem::YXmitData(uchar *buffer, int len, ZModem *info) {
  uchar hdr[3];
  uchar trail[2];
  ulong crc = 0;
  int i, err;

  hdr[0] = len == 1024 ? STX : SOH;
  hdr[1] = info->packetCount;
  hdr[2] = ~hdr[1];
  if ((err = ZXmitStr(hdr, 3, info)) || (err = ZXmitStr(buffer, len, info))) {
    return err;
  }

  if (info->PacketType == NAK) {
    /* checksum */
    for (i = 0; i < len; ++i) {
      crc += buffer[i];
    }
    trail[0] = crc % 256;
    return ZXmitStr(trail, 1, info);
  } else {
    for (i = 0; i < len; ++i) {
      crc = updcrc(buffer[i], crc);
    }
    crc = updcrc(0, crc);
    crc = updcrc(0, crc);
    trail[0] = crc / 256;
    trail[1] = crc % 256;
    return ZXmitStr(trail, 2, info);
  }
}


int FQTermZmodem::YSendFilename(ZModem *info) {
  int i, len;
  uchar obuf[1024];
  uchar *ptr = obuf;

  info->state = info->PacketType != 'G' ? YTFile : YTDataWait;
  info->packetCount = 0;
  info->offset = 0;

  i = strlen(info->rfilename);
  memcpy(ptr, info->rfilename, i + 1);
  ptr += i + 1;
  sprintf((char*)ptr, "%d %lo %o 0", info->len, info->date, info->mode);
  ptr += strlen((char*)ptr);
  *ptr++ = '\0';
  /* pad out to 128 bytes or 1024 bytes */
  i = ptr - obuf;
  len = i > 128 ? 1024 : 128;
  for (; i < len; ++i) {
    *ptr++ = '\0';
  }

  return YXmitData(obuf, len, info);
}

int FQTermZmodem::YSendData(ZModem *info) {
  int i;

  /* are there characters still in the read buffer? */

  if (info->chrCount <= 0) {
    info->bufp = 0;
    info->chrCount = fread(info->buffer, 1, info->packetsize, info->file);
    info->fileEof = feof(info->file);
  }

  if (info->chrCount <= 0) {
    fclose(info->file);
    info->state = YTEOF;
    return ZXmitStr(eotstr, 1, info);
  }

  /* pad out to 128 bytes if needed */
  if (info->chrCount < 128) {
    i = 128-info->chrCount;
    memset(info->buffer + info->bufp + info->chrCount, 0x1a, i);
    info->chrCount = 128;
  }

  info->ylen = info->chrCount >= 1024 ? 1024 : 128;
  ++info->packetCount;

  info->state = YTData;

  return YXmitData(info->buffer + info->bufp, info->ylen, info);
}


int FQTermZmodem::YSendFin(ZModem *info) {
  uchar obuf[128];

  info->state = YTFin;
  info->packetCount = 0;

  memset(obuf, 0, 128);

  return YXmitData(obuf, 128, info);
}

int FQTermZmodem::sendFilename(ZModem *info) {
  int err;
  int i;
  uchar obuf[2048];
  uchar *ptr = obuf;

  info->state = FileWait;

  if ((err = ZXmitHdr(ZFILE, ZBIN, info->fileFlags, info))) {
    return err;
  }

  i = strlen(info->rfilename);
  memcpy(ptr, info->rfilename, i + 1);
  ptr += i + 1;
  sprintf((char*)ptr, "%d %lo %o 0 %d %d 0", info->len, info->date, info->mode,
          info->filesRem, info->bytesRem);
  ptr += strlen((char*)ptr);
  *ptr++ = '\0';

  return ZXmitData(ZBIN, ptr - obuf, ZCRCW, obuf, info);
}

int FQTermZmodem::GotRinit(ZModem *info) {
  if (strFileList.count() == 0) {
    return ZmodemTFinish(info);
  }


  info->bufsize = info->hdrData[1] + info->hdrData[2] *256;
  info->zrinitflags = info->hdrData[4] + info->hdrData[3] *256;
  info->crc32 = info->zrinitflags &CANFC32;
  info->escCtrl = info->zrinitflags &ESCCTL;
  info->escHibit = info->zrinitflags &ESC8;

  /* Full streaming: If receiver can overlap I/O, and if
   * the sender can sample the reverse channel without hanging,
   * and the receiver has not specified a buffer size, then we
   * can simply blast away with ZCRCG packets.  If the receiver
   * detects an error, it sends an attn sequence and a new ZRPOS
   * header to restart the file where the error occurred.
   *
   * [note that zmodem8.doc does not define how much noise is
   * required to trigger a ZCRCW packet.  We arbitrarily choose
   * 64 bytes]
   *
   * If 'windowsize' is nonzero, and the receiver can do full
   * duplex, ZCRCQ packets are sent instead of ZCRCG, to keep track
   * of the number of characters in the queue.  If the queue fills
   * up, we pause and wait for a ZACK.
   *
   *
   * Full Streaming with Reverse Interrupt:  If sender cannot
   * sample the input stream, then we define an Attn sequence
   * that will be used to interrupt transmission.
   *
   *
   * Full Streaming with Sliding Window:  If sender cannot
   * sample input stream or respond to Attn signal, we send
   * several ZCRCQ packets until we're sure the receiver must
   * have sent back at least one ZACK.  Then we stop sending and
   * read that ZACK.  Then we send one more packet and so on.
   *
   *
   * Segmented Streaming:  If receiver cannot overlap I/O or can't do
   * full duplex and has specified a maximum receive buffer size,
   * whenever the buffer size is reached, we send a ZCRCW packet.
   *
   * TODO: what if receiver can't overlap, but hasn't set a buffer
   * size?
   *
   * ZCRCE: CRC next, frame ends, header follows
   * ZCRCG: CRC next, frame continues nonstop
   * ZCRCQ: CRC next, send ZACK, frame continues nonstop
   * ZCRCW: CRC next, send ZACK, frame ends, header follows
   */

  transferstate = transferstart;

  ZFlowControl(1, info);

  if ((info->zrinitflags &(CANFDX | CANOVIO)) == (CANFDX | CANOVIO) &&
      (SendSample || SendAttn) && info->bufsize == 0) {
    if (info->windowsize == 0) {
      info->Streaming = Full;
    } else {
      info->Streaming = StrWindow;
    }
  }

  else if ((info->zrinitflags &(CANFDX | CANOVIO)) == (CANFDX | CANOVIO) &&
           info->bufsize == 0) {
    info->Streaming = SlidingWindow;
  }

  else {
    info->Streaming = Segmented;
  }
  // get filenames to transfer
  zmodemlog("GotRinit[%s]\n", sname(info));

  if (AlwaysSinit || info->zsinitflags != 0 || info->attn != NULL) {
    SendZSInit(info);
  }


  itFile = strFileList.begin();
  QFileInfo fi(*itFile);
  FQ_TRACE("zmodem", 0) << "Number of files to be transfered: "
                        << strFileList.count();
  char *filename = strdup(fi.absoluteFilePath().toLatin1());
  char *rfilename = strdup(fi.fileName().toLatin1());
  ZmodemTFile(filename, rfilename, 0, 0, 0, 0, strFileList.count(), fi.size(),
              info);
  strFileList.erase(itFile);
  return ZmDone;
}


int FQTermZmodem::SendZSInit(ZModem *info) {
  char tmp = '\0';
  int err;
  //char	*at = (info->attn != NULL) ? info->attn : "" ;
  char *at;
  if (info->attn != NULL) {
    at = info->attn;
  } else {
    at = &tmp;
  }
  uchar fbuf[4];

  /* TODO: zmodem8.doc states: "If the ZSINIT header specifies
   * ESCCTL or ESC8, a HEX header is used, and the receiver
   * activates the specified ESC modes before reading the following
   * data subpacket." What does that mean?
   */

  zmodemlog("SendZSInit[%s]\n", sname(info));

  info->state = TInit;
  fbuf[0] = fbuf[1] = fbuf[2] = 0;
  fbuf[3] = info->zsinitflags;
  if ((err = ZXmitHdr(ZSINIT, ZBIN, fbuf, info)) || (err = ZXmitData(ZBIN,
                                                                     strlen(at) + 1, ZCRCW, (uchar*)at, info))) {
    return err;
  }
  return 0;
}


int FQTermZmodem::SendFileCrc(ZModem *info) {
  ulong crc;

  crc = FileCrc(info->filename);

  zmodemlog("SendFileCrc[%s]: %lx\n", sname(info), crc);

  return ZXmitHdrHex(ZCRC, ZEnc4(crc), info);
}

int FQTermZmodem::GotSendAck(ZModem *info) {
  ulong offset;

  offset = ZDec4(info->hdrData + 1);

  if (offset > info->lastOffset) {
    info->lastOffset = offset;
  }

  zmodemlog("GotSendAck[%s]: %lx\n", sname(info), info->offset);

  return 0; /* DONT send more data, that will happen
             * later anyway */
}

int FQTermZmodem::GotSendDoneAck(ZModem *info) {
  ulong offset;

  offset = ZDec4(info->hdrData + 1);

  if (offset > info->lastOffset) {
    info->lastOffset = offset;
  }

  zmodemlog("GotSendDoneAck[%s]: %ld\n", sname(info), info->offset);

  info->state = SendEof;
  info->timeout = 60;
  return ZXmitHdrHex(ZEOF, ZEnc4(info->offset), info);
}

int FQTermZmodem::GotSendNak(ZModem *info) {
  info->offset = info->zrposOffset;

  fseek(info->file, info->offset, 0);

  /* TODO: what if fseek fails?  Send EOF? */

  zmodemlog("GotSendNak[%s]: %ld\n", sname(info), info->offset);

  return SendMoreFileData(info);
}

int FQTermZmodem::GotSendWaitAck(ZModem *info) {

  ulong offset;
  int err;

  offset = ZDec4(info->hdrData + 1);

  FQ_TRACE("zmodem", 10) << "last = " << info->lastOffset
                         << ", now = " << offset;

  if (offset > info->lastOffset) {
    info->lastOffset = offset;
  }

  //receiver return -1 without setting this flag, kingson 00:07 14-07-04
  info->waitflag = 1;

  zmodemlog("GotSendWaitAck[%s]\n", sname(info), offset);

  if ((err = ZXmitHdr(ZDATA, ZBIN, info->hdrData + 1, info))) {
    return err;
  }
  return SendMoreFileData(info);
}

int FQTermZmodem::SkipFile(ZModem *info) {
  zmodemlog("SkipFile[%s]\n", sname(info));
  ZStatus(FileEnd, 0, info->rfilename);
  fclose(info->file);

  // stupid SMTH doesnt send further command, kick
  // lets send files in the list
  info->state = TStart;
  return GotRinit(info);
}

int FQTermZmodem::GotSendPos(ZModem *info) {
  ZStatus(DataErr, ++info->errCount, NULL);
  info->waitflag = 1; /* next pkt should wait, to resync */
  FQ_TRACE("zmodem", 9) << "GotSendPos, offset = " << info->offset;
  zmodemlog("GotSendPos[%s] %lx\n", sname(info), info->offset);
  return startFileData(info);
}

int FQTermZmodem::SendFileData(ZModem *info) {
  info->waitflag = 0;
  return startFileData(info);
}

int FQTermZmodem::ResendEof(ZModem *info) {
  return ZXmitHdrHex(ZEOF, ZEnc4(info->offset), info);
}

int FQTermZmodem::OverAndOut(ZModem *info) {
  zmodemlog("OverAndOut[%s]\n", sname(info));

  ZXmitStr((uchar*)"OO", 2, info);

  transferstate = transferstop; // transfer complete

  ZmodemReset(info); //Tranfer complete, zmodem return to receive state

  return ZmDone;
}

int FQTermZmodem::startFileData(ZModem *info) {
  int err;

  info->offset = info->lastOffset = info->zrposOffset = ZDec4(info->hdrData + 1);

  fseek(info->file, info->offset, 0);

  /* TODO: what if fseek fails?  Send EOF? */

  zmodemlog("startFileData[%s]: %lx\n", sname(info), info->offset);

  if ((err = ZXmitHdr(ZDATA, ZBIN, ZEnc4(info->offset), info))) {
    return err;
  }
  return SendMoreFileData(info);
}

uchar *FQTermZmodem::putHex(uchar *ptr, uchar c) {
  *ptr++ = hexChars[(c >> 4) &0xf];
  *ptr++ = hexChars[c &0xf];
  return ptr;
}


int FQTermZmodem::ZmodemReset(ZModem *info) {
  zmodemlog("\nZmodemReset\n");

  ZmodemRInit(info);

  return 0;
}


void FQTermZmodem::zmodemlog(const char *fmt, ...) {
  // only for debug
#ifdef FQTERM_ZMODEM_DEBUG
  va_list ap;
  struct timeval tv;
  struct tm *tm;
  static int do_ts = 1;

  zmodemlogfile = fopen("zmodem.log", "a");

  if (zmodemlogfile == NULL) {
    return ;
  }

  if (do_ts) {
    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);
    fprintf(zmodemlogfile, "%2.2d:%2.2d:%2.2d.%2.2ld: ", tm->tm_hour, tm->tm_min,
            tm->tm_sec, tv.tv_usec / 10000);
  }
  do_ts = strchr(fmt, '\n') != NULL;

  va_start(ap, fmt);
  vfprintf(zmodemlogfile, fmt, ap);
  va_end(ap);

  fclose(zmodemlogfile);
#endif

}

void FQTermZmodem::zmodemCancel() {
  ZmodemAbort(&info);
}

}  // namespace FQTerm

#include "fqterm_zmodem.moc"

