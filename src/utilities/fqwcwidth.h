// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_FQWCWIDTH_H
#define FQTERM_FQWCWIDTH_H

#include <cstdio>
#include <cstdint>
typedef uint16_t UTF16;

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
