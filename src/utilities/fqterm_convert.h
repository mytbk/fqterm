// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_CONVERT_H
#define FQTERM_CONVERT_H

namespace FQTerm {

class FQTermConvert {
 public:
  FQTermConvert();
  ~FQTermConvert();

  char *G2B(const char *string, int length);
  char *B2G(const char *string, int length);

 private:
  void g2b(unsigned char c1, unsigned char c2, char *s);
  void b2g(unsigned char c1, unsigned char c2, char *s);

  static unsigned char GtoB[];
  static unsigned char BtoG[];
};

}  // namespace FQTerm

#endif  // FQTERM_CONVERT_H
