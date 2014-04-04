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

#ifndef FQTERM_COMMON_H
#define FQTERM_COMMON_H

#if defined(WIN32)
#define OS_NEW_LINE "\r\n"
#elif defined(__APPLE__) 
#define OS_NEW_LINE "\r"
#else
#define OS_NEW_LINE "\n"
#endif

class QString;

namespace FQTerm {

enum Directions {
    kHome = 0,
    kEnd = 1,
    kPageUp = 2,
    kPageDown = 3,
    kUp = 4,
    kDown = 5,
    kLeft = 6,
    kRight = 7
};

const char * const kDirections[] =  {
  // 4
  "\x1b[1~",  // 0 HOME
  "\x1b[4~",  // 1 END
  "\x1b[5~",  // 2 PAGE UP
  "\x1b[6~",  // 3 PAGE DOWN
  // 3
  "\x1b[A",  // 4 UP
  "\x1b[B",  // 5 DOWN
  "\x1b[D",  // 6 LEFT
  "\x1b[C"   // 7 RIGHT
};

void runProgram(const QString &cmd, const QString& arg = "", bool bg = true);

}  // namespace FQTerm

#endif  // FQTERM_COMMON_H
