// SPDX-License-Identifier: GPL-2.0-or-later

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
