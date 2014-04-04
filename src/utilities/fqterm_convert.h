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
