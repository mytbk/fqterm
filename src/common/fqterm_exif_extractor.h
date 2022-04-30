// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_EXIF_EXTRACTOR
#define FQTERM_EXIF_EXTRACTOR

#include "fqterm_trace.h"

#include <map>
#include <string>
#include <cstdint>

namespace FQTerm {

class ExifExtractor
{
public:

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
  uint32_t ifdOffset_;

  static const uint16_t IFD0Tag[16];
  static const char IFD0TagName[16][30];
  static const uint16_t SubIFDTag[38];
  static const char SubIFDTagName[38][30];
  static const char exifHeaderStandard[6];

  std::string exifInformation_;
};



} //namespace FQTerm

#endif
