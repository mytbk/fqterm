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

#ifndef FQTERM_EXIF_EXTRACTOR
#define FQTERM_EXIF_EXTRACTOR

//#include <QtGlobal>

#include "fqterm_trace.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <map>
#include <string>
//using namespace std;
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <cstdint>
#endif

namespace FQTerm {

class ExifExtractor
{
public:

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    typedef std::uint32_t uint32;
    typedef std::uint16_t uint16;
    typedef std::int32_t int32;
    typedef std::int16_t int16;
#elif defined(QT_VERSION)
  typedef quint32 uint32;
  typedef quint16 uint16;
  typedef qint32 int32;
  typedef qint16 int16;
#else
  typedef unsigned int uint32;
  typedef unsigned short int uint16;
  typedef short int int16;
  typedef int int32;
#endif

  enum DATATYPE {ZEORGARD = 0, UNSIGNEDCHAR = 1, ASCIISTRING = 2, UNSIGNEDINT16 = 3, UNSIGNEDINT32 = 4, 
    UNSIGNEDRATIONAL = 5, SIGNEDCHAR = 6, UNDEFINED = 7, SIGNEDSHORT = 8, SIGNEDLONG = 9, RATIONAL = 10,
    SIGNEDFLOAT = 11, DOUBLEFLOAT = 12, MAXGUARD = 13};


  ExifExtractor();
  ~ExifExtractor();
  std::string extractExifInfo(std::string fileName);
  std::string extractExifInfo(FILE* file);

  std::string info();

  std::string operator[](const std::string& key);

private:
  void postRead();

  std::string readInfo();
  bool analyzeIFD0TagInfo();

  bool analyzeSubIFDTagInfo();

  bool readIFD0();
  bool readSubIFD();
  bool isReadable();
  bool checkEndian();
  void toggleEndian(void* buf, size_t s);

  void reset();

  bool read(void* buffer, size_t elementSize, size_t count);

  void seek(long offset, int origin);

  void rewind();

  template<class T>
  T gcd(T a, T b) {
    if (a == 0 || b == 0)
      return a + b;
    return gcd(b, a % b);
  }


  bool toggle_;
  std::map<std::string, std::string> exifKeyValuePairs_;
  FILE* exifFile_;
  uint32 ifdOffset_;
  uint32 subIfdOffset_;

  static const uint16 IFD0Tag[16];
  static const char IFD0TagName[16][30];
  static const uint16 SubIFDTag[38];
  static const char SubIFDTagName[38][30];
  static const char exifHeaderStandard[6];

  std::string exifInformation_;
};



} //namespace FQTerm

#endif
