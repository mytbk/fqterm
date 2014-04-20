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

#include <QString>
#include <QRegExp>
#include <QDir>
#include <QStringList>
#include <ctype.h>

#ifdef Q_OS_WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "fqterm_trace.h"
#include "fqterm_ip_location.h"
#include "fqterm_path.h"
#include "fqterm.h"

namespace FQTerm {

FQTermIPLocation* FQTermIPLocation::instance_ = NULL;

FQTermIPLocation::FQTermIPLocation(const QString &pathLib) {
  ipDatabase_ = new IPDatabase;
  ipDatabase_->ipfp = NULL;

  isFileExiting_ = true;

  //case-insensitive match
  QDir dir(pathLib);
  QStringList files = dir.entryList(QStringList(
                                        "[Qq][Qq][Ww][Rr][Yy].[Dd][Aa][Tt]"), QDir::Files);
  if (!files.isEmpty()) {
    if ((ipDatabase_->ipfp = fopen((pathLib + (files.at(0))).toLocal8Bit(), "rb")) == NULL) {
      FQ_TRACE("iplocation", 0) << "Can't open ip database file!";
      isFileExiting_ = false;
    }
  } else {
    isFileExiting_ = false;
  }

}

FQTermIPLocation::~FQTermIPLocation() {
  if (ipDatabase_->ipfp != NULL) {
    fclose(ipDatabase_->ipfp);
  }
  delete ipDatabase_;
}

uint32 FQTermIPLocation::byteArrayToInt(char *ip, int count) {
  uint32 tmp, ret = 0L;
  if (count < 1 || count > 4) {
    FQ_TRACE("iplocation", 0) << "byteArrayToInt() error!";
    return 0;
  }
  for (int i = 0; i < count; i++) {
    tmp = ((uint32)ip[i]) &0x000000FF;
    ret |= (tmp << (8 *i));
  }
  return ret;
}

void FQTermIPLocation::readFrom(FILE *fp, uint32 offset, char *buf, int len) {
  if (fseek(fp, (long)offset, SEEK_SET) == -1) {
    FQ_TRACE("iplocation", 0) << "readFrom() error 1";
    memset(buf, 0, len);
    return ;
  }
  if (fread(buf, sizeof(char), len, fp) == 0) {
    FQ_TRACE("iplocation", 0) << "readFrom() error 2";
    memset(buf, 0, len);
    return ;
  }
  return ;
}

int FQTermIPLocation::readLineFrom(FILE *fp, uint32 offset, QByteArray &ret_str) {
  char str[512];
  if (fseek(fp, (long)offset, SEEK_SET) == -1) {
    FQ_TRACE("iplocation", 0) << "readLineFrom error 1";
    ret_str = QByteArray();
    return -1;
  }
  if (fgets((char*)str, 512, fp) == NULL) {
    FQ_TRACE("iplocation", 0) << "readLineFrom error 2";
    ret_str = QByteArray();
    return -1;
  }
  ret_str = QByteArray(str);
  return (ret_str.length());
}

uint32 FQTermIPLocation::getString(FILE *fp, uint32 offset, uint32 lastoffset,
                                   QByteArray &ret, unsigned int flag, int maxRecursiveDepth) {
  if (maxRecursiveDepth <= 0)
    return 0;
  unsigned int fg;
  if (fp == NULL) {
    return 0;
  }
  char buf[3] = {0};
  readFrom(fp, offset, buf, 1);
  if (buf[0] == 0x01 || buf[0] == 0x02) {
    fg = buf[0];
    readFrom(fp, offset + 1, buf, 3);
    return getString(fp, byteArrayToInt(buf, 3), offset, ret, fg, maxRecursiveDepth - 1);
  } else {
    readLineFrom(fp, offset, ret);
  }
  switch (flag) {
    case 0x01:
      return 0;
    case 0x02:
      return lastoffset + 4;
    default:
      return offset + ret.length() + 1;
  }
}


void FQTermIPLocation::getCountryCity(FILE *fp, uint32 offset, QByteArray &country,
                                      QByteArray &city) {
  uint32 next_offset;
  if (fp == NULL) {
    return ;
  }
  next_offset = getString(fp, offset, 0L, country, 0);
  if (next_offset == 0) {
    city = "";
  } else {
    getString(fp, next_offset, 0L, city, 0);
  }
  return ;
}

void FQTermIPLocation::setIpRange(int rec_no, IPDatabase *f) {
  uint32 offset;
  if (f == NULL) {
    return ;
  }
  char buf[7] = {0};
  offset = f->offset_first_start_ip + rec_no * 7;
  readFrom(f->ipfp, offset, buf, 7);
  f->cur_start_ip = byteArrayToInt(buf, 4);
  f->offset_cur_end_ip = byteArrayToInt(buf + 4, 3);

  readFrom(f->ipfp, f->offset_cur_end_ip, buf, 4);
  f->cur_end_ip = byteArrayToInt(buf, 4);

}

bool FQTermIPLocation::getLocation(QString &url, QString &country, QString &city)
{
  QByteArray gbCountry, gbCity;
  bool result = getLocation(url, gbCountry, gbCity);
  country = encoding2unicode(gbCountry, FQTERM_ENCODING_GBK);
  city = encoding2unicode(gbCity, FQTERM_ENCODING_GBK);
  return result;
}

bool FQTermIPLocation::getLocation(QString &url, QByteArray &country,
                                   QByteArray &city) {
  int rec, record_count, B, E;
  uint32 ip;
#ifdef Q_OS_WIN32
  uint32 ipValue = inet_addr((const char*)url.toLatin1());
#else
  in_addr_t ipValue = inet_addr((const char*)url.toLatin1());
#endif
  if ((int)ipValue == -1) {
    return false;
  } else {
    ip = ntohl(ipValue);
  }

  char buf[4] = {0};
  readFrom(ipDatabase_->ipfp, 0L, (char*)buf, 4);
  ipDatabase_->offset_first_start_ip = byteArrayToInt((char*)buf, 4);
  readFrom(ipDatabase_->ipfp, 4L, (char*)buf, 4);
  ipDatabase_->offset_last_start_ip = byteArrayToInt((char*)buf, 4);

  record_count = (ipDatabase_->offset_last_start_ip - ipDatabase_->offset_first_start_ip) / 7;
  if (record_count <= 1) {
    return false;
  }

  // search for right range
  B = 0;
  E = record_count;
  while (B < E-1) {
    rec = (B + E) / 2;
    setIpRange(rec, ipDatabase_);
    if (ip == ipDatabase_->cur_start_ip) {
      B = rec;
      break;
    }
    if (ip > ipDatabase_->cur_start_ip) {
      B = rec;
    } else {
      E = rec;
    }
  }
  setIpRange(B, ipDatabase_);

  if (ipDatabase_->cur_start_ip <= ip && ip <= ipDatabase_->cur_end_ip) {
    getCountryCity(ipDatabase_->ipfp, ipDatabase_->offset_cur_end_ip + 4, country, city);
    //country.replace( country.find( "CZ88.NET", 0, false ), 8, "" );
    if ((rec = country.toUpper().indexOf("CZ88.NET", 0)) >= 0) {
      country.replace(rec, 8, "********");
    }
    if ((rec = city.toUpper().indexOf("CZ88.NET", 0)) >= 0) {
      city.replace(rec, 8, "********");
    }

  } else {
    // not in this range... miss
    country = "unkown";
    city = "";
  }
  // if ip_start<=ip<=ip_end
  return true;
}

FQTermIPLocation* FQTermIPLocation::getInstance() {
  if (instance_ == NULL) {
    instance_ = new FQTermIPLocation(getPath(USER_CONFIG));
    if (instance_->isFileExiting_ == false) {
      delete instance_;
      instance_ = new FQTermIPLocation(getPath(RESOURCE));
      if (instance_->isFileExiting_ == false) {
        delete instance_;
        instance_ = NULL;
      }
    }
  }
  return instance_;
}

void FQTermIPLocation::Destroy() {
  delete instance_;
  instance_ = NULL;
}

}  // namespace FQTerm
