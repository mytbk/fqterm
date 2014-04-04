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

#ifndef FQTERM_FQWCWIDTH_H
#define FQTERM_FQWCWIDTH_H

#ifdef WIN32
#define uint32_t unsigned __int32
#define uint16_t unsigned __int16
typedef wchar_t UTF16;
#else  // WIN32
#include <stdio.h>
#include <stdint.h>
typedef uint16_t UTF16;
#endif  // WIN32

typedef char Verify_UTF16_Is_Two_Bytes[
    (sizeof(UTF16) == 2)? 1 : -1];

namespace FQTerm {

//const uint16_t URC = 0xfffd; // the Unicode Replacement Character
// TODO_UTF16: 0xfffd is of width 2, which is not desired.
const uint16_t URC = '?'; // use '?' instead of the Unicode Replacement Character

const int MAX_CELLS_PER_CHAR = 2;

int get_str_width(uint32_t ucs);
int get_str_width(const uint32_t *pwcs, size_t n);
int get_str_width(const UTF16 *utf16_str, size_t n);
int get_str_width(const UTF16 *utf16_str, size_t n, int max_width, int &element_consumed);

// Move p to next code point in max_n UTF16 characters.
// Return the width of the current code point.
int mk_advance_one_code_point(const UTF16 **p, const UTF16 *end);

// Move p to next code point in max_n UTF16 characters.
// Return the width.
int mk_advace_at_least_one_cell(const UTF16 **p, const UTF16 *end);

}  // namespace FQTerm 

#endif  // FQTERM_FQWCWIDTH_H
