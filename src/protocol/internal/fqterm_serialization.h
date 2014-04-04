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

#ifndef FQTERM_SERIALIZATION_H
#define FQTERM_SERIALIZATION_H

#if defined(WIN32)
typedef unsigned __int16 u_int16_t;
#else 
#include <arpa/inet.h>
#endif

namespace FQTerm {

inline u_int16_t ntohu16(const unsigned char *buf) {
  return (buf[0] << 8) | buf[1];
}

inline u_int32_t ntohu32(const unsigned char *buf) {
  return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

inline void htonu32(unsigned char *buf, u_int32_t number) {
  buf[0] = (number >> 24) & 0xFF;
  buf[1] = (number >> 16) & 0xFF;
  buf[2] = (number >> 8) & 0xFF;
  buf[3] = number & 0xFF;
}

inline void htonu16(unsigned char *buf, u_int16_t number) {
  buf[0] = (number >> 8 & 0xFF);
  buf[1] = (number & 0xFF);
}

}  // namespace FQTerm

#endif  // FQTERM_SERIALIZATION_H
