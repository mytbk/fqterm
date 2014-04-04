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

#include "fqterm_exif_extractor.h"

namespace FQTerm {

const ExifExtractor::uint16 ExifExtractor::IFD0Tag[16] = {0x010e, 0x010f, 0x0110, 0x0112, 0x011a,
  0x011b, 0x0128, 0x0131, 0x0132, 0x013e,
  0x013f, 0x0211, 0x0213, 0x0214, 0x8298, 0x8769
};

const char ExifExtractor::IFD0TagName[16][30] = {  "ImageDescription", "Make", "Model", "Orientation",
  "XResolution", "YResolution", "ResolutionUnit", "Software",
  "DateTime", "WhitePoint", "PrimaryChromaticities", "YCbCrCoefficients",
  "YCbCrPositioning", "ReferenceBlackWhite", "Copyright", "ExifOffset"
};

const ExifExtractor::uint16 ExifExtractor::SubIFDTag[38] = {0x829a,0x829d,0x8822,0x8827,0x9000,0x9003,0x9004,0x9101,0x9102,0x9201,0x9202,0x9203,0x9204,
  0x9205,0x9206,0x9207,0x9208,0x9209,0x920a,0x927c,0x9286,0x9290,0x9291,0x9292,0xa000,0xa001,
  0xa002,0xa003,0xa004,0xa005,0xa20e,0xa20f,0xa210,0xa215,0xa217,0xa300,0xa301,0xa302};

const char ExifExtractor::SubIFDTagName[38][30] = {"ExposureTime","FNumber","ExposureProgram","ISOSpeedRatings","ExifVersion","DateTimeOriginal","DateTimeDigitized",
  "ComponentsConfiguration","CompressedBitsPerPixel","ShutterSpeedValue","ApertureValue","BrightnessValue","ExposureBiasValue",
  "MaxApertureValue","SubjectDistance","MeteringMode","LightSource","Flash","FocalLength","MakerNote","UserComment","SubsecTime",
  "SubsecTimeOriginal","SubsecTimeDigitized","FlashPixVersion","ColorSpace","ExifImageWidth","ExifImageHeight","RelatedSoundFile",
  "ExifInteroperabilityOffset","FocalPlaneXResolution","FocalPlaneYResolution","FocalPlaneResolutionUnit","ExposureIndex",
  "SensingMethod","FileSource","SceneType","CFAPattern"};

const char ExifExtractor::exifHeaderStandard[6] = {'E', 'x', 'i', 'f', '\0', '\0'};

ExifExtractor::ExifExtractor()
{
  exifFile_ = NULL;
  reset();
}

ExifExtractor::~ExifExtractor()
{
  reset();
}

std::string ExifExtractor::extractExifInfo(std::string fileName) {
  return extractExifInfo(fopen(fileName.c_str(), "rb"));
}

std::string ExifExtractor::extractExifInfo(FILE* file) {
  reset();
  exifFile_ = file;
  if (!exifFile_ || !isReadable() || !checkEndian()) {
    return exifInformation_;
  }
  if (!readIFD0())
    return exifInformation_;
  readSubIFD();
  postRead();
  std::map<std::string, std::string>::iterator ii;
  for (ii = exifKeyValuePairs_.begin();
    ii != exifKeyValuePairs_.end();
    ++ii) {
      if ((*ii).first == "UserComment") {
        continue;
      }
      exifInformation_ += (*ii).first + " : " + (*ii).second + "\n";
  }
  fclose(exifFile_);
  exifFile_ = NULL;
  return exifInformation_;
}
std::string ExifExtractor::info() {
  return exifInformation_;
}

std::string ExifExtractor::operator[]( const std::string& key ) {
  std::map<std::string, std::string>::iterator ii;
  if ((ii = exifKeyValuePairs_.find(key)) != exifKeyValuePairs_.end()) {
    return (*ii).second;
  }
  return "";
}

void ExifExtractor::postRead() {
  std::map<std::string, std::string>::iterator ii;
  if ((ii = exifKeyValuePairs_.find("Orientation")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      switch((*ii).second[0]) {
        case '1': (*ii).second = "1st row - 1st col : top - left side"; break;
        case '2': (*ii).second = "1st row - 1st col : top - right side"; break;
        case '3': (*ii).second = "1st row - 1st col : bottom - right side"; break;
        case '4': (*ii).second = "1st row - 1st col : bottom - left side"; break;
        case '5': (*ii).second = "1st row - 1st col : left side - top"; break;
        case '6': (*ii).second = "1st row - 1st col : right side - top"; break;
        case '7': (*ii).second = "1st row - 1st col : right side - bottom"; break;
        case '8': (*ii).second = "1st row - 1st col : left side - bottom"; break;
        default:  exifKeyValuePairs_.erase(ii); break;

      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("ResolutionUnit")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      switch((*ii).second[0]) {
        case '1': (*ii).second = "no-unit"; break;
        case '2': (*ii).second = "inch"; break;
        case '3': (*ii).second = "centimeter"; break; 
        default:  (*ii).second = "inch"; break;
      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("DateTime")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      for (size_t i = 0; i < (*ii).second.length() && i < 11; ++i) {
        if ((*ii).second[i] == ':'){
          (*ii).second[i] = '.';
        }
      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("DateTimeOriginal")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      for (size_t i = 0; i < (*ii).second.length() && i < 11; ++i) {
        if ((*ii).second[i] == ':'){
          (*ii).second[i] = '.';
        }
      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("DateTimeDigitized")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      for (size_t i = 0; i < (*ii).second.length() && i < 11; ++i) {
        if ((*ii).second[i] == ':'){
          (*ii).second[i] = '.';
        }
      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("YCbCrPositioning")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      switch((*ii).second[0]) {
        case '1': (*ii).second = "The center of pixel array"; break;
        case '2': (*ii).second = "The datum point"; break;
        default:  exifKeyValuePairs_.erase(ii); break;
      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("MeteringMode")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      uint16 value = 123;
      sscanf((*ii).second.c_str(), "%hu", &value);
      switch(value) {
        case 0: (*ii).second = "Unknown"; break;
        case 1: (*ii).second = "Average"; break;
        case 2: (*ii).second = "CenterWeightedAverage"; break;
        case 3: (*ii).second = "Spot"; break;
        case 4: (*ii).second = "Multi-spot"; break;
        case 5: (*ii).second = "Pattern"; break;
        case '6': (*ii).second = "Partial"; break;
        case 255: (*ii).second = "Other"; break;
        default:  exifKeyValuePairs_.erase(ii); break;

      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("LightSource")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      uint16 value = 123;
      sscanf((*ii).second.c_str(), "%hu", &value);
      switch(value) {
        case 0: (*ii).second = "unknown"; break;
        case 1: (*ii).second = "daylight"; break;
        case 2: (*ii).second = "fluorescent"; break;
        case 3: (*ii).second = "tungsten"; break;
        case 10: (*ii).second = "flash"; break;
        case 17: (*ii).second = "standard light A"; break;
        case 18: (*ii).second = "standard light B"; break;
        case 19: (*ii).second = "standard light C"; break;
        case 20: (*ii).second = "D55"; break;
        case 21: (*ii).second = "D65"; break;
        case 22: (*ii).second = "D75"; break;
        case 255: (*ii).second = "other"; break;
        default:  exifKeyValuePairs_.erase(ii); break;

      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("Flash")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      uint16 value = 123;
      sscanf((*ii).second.c_str(), "%hu", &value);
      value &= 0x07;
      switch(value) {
        case 0: (*ii).second = "Flash did not fire"; break;
        case 1: (*ii).second = "Flash fired"; break;
        case 5: (*ii).second = "Flash fired but strobe return light not detected"; break;
        case 7: (*ii).second = "Flash fired and strobe return light detected"; break;
        default:  exifKeyValuePairs_.erase(ii); break;
      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("ColorSpace")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      switch((*ii).second[0]) {
        case '1': (*ii).second = "sRGB color space"; break;
        case '6': (*ii).second = "Uncalibrated"; break;
        default:  exifKeyValuePairs_.erase(ii); break;
      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("FocalPlaneResolutionUnit")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      switch((*ii).second[0]) {
        case '1': (*ii).second = "no-unit"; break;
        case '2': (*ii).second = "inch"; break;
        case '3': (*ii).second = "centimeter"; break; 
        default:  exifKeyValuePairs_.erase(ii); break;
      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("SensingMethod")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      switch((*ii).second[0]) {
        case '2': (*ii).second = "1 chip color area sensor"; break;
        default:  (*ii).second = "other"; break;
      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("ExifVersion")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() != 4) {
      exifKeyValuePairs_.erase(ii);
    } else {
      (*ii).second = 'V' + (*ii).second.substr(0, 2) + '.' + (*ii).second.substr(2);
    }
  }
  if ((ii = exifKeyValuePairs_.find("ComponentsConfiguration")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() != 4) {
      exifKeyValuePairs_.erase(ii);
    } else {
      static const char CCName[7][10] = {"", "[Y]", "[Cb]", "[Cr]", "[Red]", "[Green]", "[Blue]"};
      std::string cc;
      for(int i = 0; i < 4; ++i) {
        int index = (*ii).second[i];
        if (index < 0 || index >= 7) {
          index = 0;
        }
        cc += CCName[index];
      }
    }
  }
  if ((ii = exifKeyValuePairs_.find("ExifVersion")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() != 4) {
      exifKeyValuePairs_.erase(ii);
    } else {
      (*ii).second = 'V' + (*ii).second.substr(0, 2) + '.' + (*ii).second.substr(2);
    }
  }
  if ((ii = exifKeyValuePairs_.find("MakerNote")) != exifKeyValuePairs_.end()) {
    exifKeyValuePairs_.erase(ii);
  }
  if ((ii = exifKeyValuePairs_.find("CFAPattern")) != exifKeyValuePairs_.end()) {
    exifKeyValuePairs_.erase(ii);
  }
  if ((ii = exifKeyValuePairs_.find("UserComment")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() < 8) {
      exifKeyValuePairs_.erase(ii);
    } else {
      //Leave it to higher level which can deal with encoding.
      //"\0x41\0x53\0x43\0x49\0x49\0x00\0x00\0x00"//ASCII
      //"\0x4a\0x49\0x53\0x00\0x00\0x00\0x00\0x00"//JIS
      //"\0x55\0x4e\0x49\0x43\0x4f\0x44\0x45\0x00"//Unicode
      //"\0x00\0x00\0x00\0x00\0x00\0x00\0x00\0x00"//Undefined
    }
  }
  if ((ii = exifKeyValuePairs_.find("FlashPixVersion")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() != 4) {
      exifKeyValuePairs_.erase(ii);
    } else {
      (*ii).second = 'V' + (*ii).second.substr(0, 2) + '.' + (*ii).second.substr(2);
    }
  }  
  //FileSource
  if ((ii = exifKeyValuePairs_.find("FileSource")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() != 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      if ((*ii).second[0] == '\003') {
        (*ii).second = "Digital still camera";
      } else {
        exifKeyValuePairs_.erase(ii);
      }
    }
  }  
  if ((ii = exifKeyValuePairs_.find("SceneType")) != exifKeyValuePairs_.end()) {
    if ((*ii).second.length() != 1) {
      exifKeyValuePairs_.erase(ii);
    } else {
      if ((*ii).second[0] == '\001') {
        (*ii).second = "Directly photographed";
      } else {
        exifKeyValuePairs_.erase(ii);
      }
    } 
  }

}

std::string ExifExtractor::readInfo() {
  uint32 count;
  uint32 offset;
  uint16 shortValue;
  uint32 longValue;
  uint32 urationalp;
  uint32 urationalc;
  uint32 urationalgcd;
  int32 rationalp;
  int32 rationalc;
  int32 rationalgcd;
  int32 sign;
  DATATYPE dataType;
  char* buffer;
  std::string ret = "";

  if (!read(&shortValue, 2,1) || shortValue == 0 || shortValue > MAXGUARD) {
    return ret;
  }
  dataType = (DATATYPE)shortValue;

  if (!read(&count, 4,1)) {
    return ret;
  }

  switch(dataType) {
    case UNDEFINED:
      buffer = new char[count + 1];
      
      if (count < 4) {
        if (!read(buffer, 1, count)) {
          return ret;
        }
      } else {
        if (!read(&offset, 4, 1)) {
          return ret;
        }
        seek(12 + offset, SEEK_SET);
        memset(buffer, 0, count + 1);
        read(buffer, 1, count);
      }
      ret = std::string(buffer, count);
      delete []buffer;
      break;
    case ASCIISTRING:
      if (!read(&offset, 4, 1)) {
        break;
      }
      seek(12 + offset, SEEK_SET);
      buffer = new char[count + 1];
      memset(buffer, 0, count + 1);
      read(buffer, 1, count);
      ret = buffer;
      delete []buffer;
      break;
    case UNSIGNEDINT16:
      if (!read(&shortValue, 2, 1)) {
        break;
      }
      buffer = new char[256];
      sprintf(buffer, "%hu", shortValue);
      ret = buffer;
      delete []buffer;
      break;
    case UNSIGNEDINT32:
      if (!read(&longValue, 4, 1)) {
        break;
      }
      buffer = new char[256];
      sprintf(buffer, "%u", longValue);
      ret = buffer;
      delete []buffer;
      
      break;
    case UNSIGNEDRATIONAL:
      if (!read(&offset, 4, 1)) {
        break;
      }
      seek(12 + offset, SEEK_SET);
      if (!read(&urationalc, 4, 1)) {
        break;
      }
      if (!read(&urationalp, 4, 1)) {
        break;
      }
      urationalgcd = gcd(urationalp, urationalc);
      if (urationalgcd == 0) urationalgcd = 1;
      buffer = new char[256];
      sprintf(buffer, "%u/%u", urationalc / urationalgcd, urationalp / urationalgcd);
      ret = buffer;
      delete []buffer;
      break;
    case RATIONAL:
      if (!read(&offset, 4, 1)) {
        break;
      }
      sign = 1;
      seek(12 + offset, SEEK_SET);
      if (!read(&rationalc, 4, 1)) {
        break;
      }
      if (rationalc < 0) {
        sign *= -1;
        rationalc = -rationalc;
      }
      if (!read(&rationalp, 4, 1)) {
        break;
      }
      if (rationalp < 0) {
        sign *= -1;
        rationalp = -rationalp;
      }
      rationalgcd = gcd(rationalp, rationalc);
      if (rationalgcd == 0) rationalgcd = 1;
      buffer = new char[256];
      sprintf(buffer, "%d/%d", sign * rationalc / rationalgcd, rationalp / rationalgcd);
      ret = buffer;
      delete []buffer;
      break;
    default:
      break;
  }
  return ret;
}

bool ExifExtractor::analyzeIFD0TagInfo() {
  bool res = true;
  uint16 tag;
  if (!read(&tag, 2, 1))
    res = false;
  int index;
  for (index = 0; index < 16; ++index) {
    if (IFD0Tag[index] == tag) {
      break;
    }
  }
  if (index == 16) {
    res = false;
  }
  long pos = ftell(exifFile_) + 10;

  if (res) {
    std::string info = readInfo();
    if (info != "")
      exifKeyValuePairs_[IFD0TagName[index]] = info;
  }

  seek(pos, SEEK_SET);
  return res;
}

bool ExifExtractor::analyzeSubIFDTagInfo() {
  bool res = true;
  uint16 tag;
  if (!read(&tag, 2, 1))
    res = false;
  int index;
  for (index = 0; index < 38; ++index) {
    if (SubIFDTag[index] == tag) {
      break;
    }
  }
  if (index == 38) {
    res = false;
  }
  long pos = ftell(exifFile_) + 10;

  if (res) {
    std::string info = readInfo();
    if (info != "")
      exifKeyValuePairs_[SubIFDTagName[index]] = info;
  }

  seek(pos, SEEK_SET);
  return res;
}

bool ExifExtractor::readIFD0() {
  seek(16, SEEK_SET);
  if (!read(&ifdOffset_, 4, 1)) {
    return false;
  }
  seek(ifdOffset_ + 12, SEEK_SET);
  uint16 entryCount = 0;
  if (!read(&entryCount, 2, 1)) {
    return false;
  }
  int app1Entry = entryCount;
  while (app1Entry >= 0) {
    analyzeIFD0TagInfo();
    app1Entry--;
  }
  return true;
}

bool ExifExtractor::readSubIFD() {
  std::map<std::string, std::string>::iterator ii;
  if ((ii = exifKeyValuePairs_.find("ExifOffset")) == exifKeyValuePairs_.end()) {
    return false;
  }
  uint32 exifOffset = 0;
  sscanf((*ii).second.c_str(), "%u", &exifOffset);
  seek(12 + exifOffset, SEEK_SET);

  uint16 entryCount = 0;
  if (!read(&entryCount, 2, 1)) {
    return false;
  }
  int app1Entry = entryCount;
  while (app1Entry >= 0) {
    analyzeSubIFDTagInfo();
    app1Entry--;
  }
  return true;
}

bool ExifExtractor::isReadable() {
  rewind();
  unsigned char soi[2];
  if (!read(soi, 2, 1))
    return false;
  if (soi[0] != 0xFF || soi[1] != 0xD8) {
    return false;
  }
  unsigned char app1[2];
  if (!read(app1, 2, 1))
    return false;
  if (app1[0] != 0xFF || app1[1] != 0xE1) {
    return false;
  }
  seek(2, SEEK_CUR);
  char exifHeader[6];
  if (!read(exifHeader, 1, 6)) {
    return false;
  }
  for (int i = 0; i < 6; ++i) {
    if (exifHeader[i] != exifHeaderStandard[i])
      return false;
  }
  return true;
}

bool ExifExtractor::checkEndian() {
  toggle_ = false;
  seek(12, SEEK_SET);
  uint16 endian = 0;
  if (!read(&endian, 2, 1)) {
    return false;
  }
  if (endian == 0x4949) {
    //II
    toggle_ = false;
  }
  else if (endian == 0x4d4d) {
    //MM
    toggle_ = true;
  }
  return true;
}

void ExifExtractor::toggleEndian( void* buf, size_t s ) {
  char* buffer = (char*)buf;
  if (!toggle_) {
    return;
  }
  for (size_t i = 0; i < s / 2; ++i) {
    char tmp;
    tmp = buffer[i];
    buffer[i] = buffer[s - i - 1];
    buffer[s - i - 1] = tmp;
  }
}

void ExifExtractor::reset() {
  toggle_ = false;
  exifKeyValuePairs_.clear();
  if (exifFile_) {
    fclose(exifFile_);
    exifFile_ = NULL;
  }
  ifdOffset_ = 0;
  exifInformation_.clear();
}

bool ExifExtractor::read( void* buffer, size_t elementSize, size_t count ) {
  for (size_t i = 0; i < count; ++i) {
    if (fread((char*)buffer + i * elementSize, elementSize, 1, exifFile_) != 1)
      return false;
    toggleEndian((char*)buffer + i * elementSize, elementSize);
  }
  return true;
}

void ExifExtractor::seek( long offset, int origin ) {
  fseek(exifFile_, offset, origin);
}

void ExifExtractor::rewind() {
  ::rewind(exifFile_);
}
} //namespace FQTerm
