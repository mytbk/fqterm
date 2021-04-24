// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_IPLOCATION_H
#define FQTERM_IPLOCATION_H

#include <stdio.h>
#include <stdlib.h>

class QString;

namespace FQTerm {

#define MAX_RECURSIVE_IP_DATA_DEPTH 100

typedef unsigned long uint32;


class FQTermIPLocation {
public:
  static FQTermIPLocation* getInstance();
  static void Destroy();

  bool getLocation(QString &url, QString &country, QString &city);
  bool getLocation(QString &url, QByteArray &country, QByteArray &city);

private:
  struct IPDatabase {
    uint32 offset_first_start_ip; // first abs offset of start ip
    uint32 offset_last_start_ip; // last abs offset of start ip
    uint32 cur_start_ip; // start ip of current search range
    uint32 cur_end_ip; // end ip of current search range
    uint32 offset_cur_end_ip; // where is the current end ip saved
    FILE *ipfp; // IO Channel to read file
  };

  FQTermIPLocation(const QString &pathLib);
  ~FQTermIPLocation();
  uint32 byteArrayToInt(char *ip, int count);
  void readFrom(FILE *fp, uint32 offset, char *buf, int len);
  int readLineFrom(FILE *fp, uint32 offset, QByteArray &ret_str);
  uint32 getString(FILE *fp, uint32 offset, uint32 lastoffset, QByteArray &str,
    unsigned int flag, int maxRecursiveDepth = MAX_RECURSIVE_IP_DATA_DEPTH);
  void getCountryCity(FILE *fp, uint32 offset, QByteArray &country, QByteArray &city);
  void setIpRange(int rec_no, IPDatabase *f);


  IPDatabase *ipDatabase_;
  bool isFileExiting_;

  static FQTermIPLocation* instance_;
};

}  // namespace FQTerm

#endif  // FQTERM_IPLOCATION_H
