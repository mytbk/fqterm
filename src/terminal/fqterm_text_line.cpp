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

#include <stdio.h>
#include <QRegExp>
#include <QString>

#include "fqterm.h"
#include "fqterm_text_line.h"

namespace FQTerm {

#ifndef NDEBUG
//#define TEXT_LINE_CHECK
#else
//#define TEXT_LINE_CHECK
#endif 

#ifndef TEXT_LINE_CHECK
void FQTermTextLine::verifyTextLine() const {}
void FQTermTextLine::verifyCellRange(unsigned, unsigned) const {}
#else
void FQTermTextLine::verifyTextLine() const {
  unsigned last_cell = 0;
  unsigned cur_cell = last_cell;

  if (cells_.size() == 0 
      || cell_colors_.size() == 0
      || cell_attrs_.size() == 0
      || chars_.size() == 0
      ) {
    FQ_VERIFY(cells_.size() == 0); 
    FQ_VERIFY(cell_colors_.size() == 0);
    FQ_VERIFY(cell_attrs_.size() == 0);
    FQ_VERIFY(chars_.size() == 0);
    return;
  }

  FQ_VERIFY(max_cell_count_ < 65536);
  FQ_VERIFY(chars_.size() < 65536);
  FQ_VERIFY(cells_.size() <= max_cell_count_);
  FQ_VERIFY(cells_.size() == cell_colors_.size());
  FQ_VERIFY(cells_.size() == cell_attrs_.size());

  const UTF16 *start = &chars_[0];
  const UTF16 *end = start + chars_.size();

  for (const UTF16 *p = start; p < end;) {
    int w = mk_advace_at_least_one_cell(&p, end);

    FQ_VERIFY(w >= 0); // should be valid unicode string.

    cur_cell += w;

    for (unsigned i = last_cell; i < cur_cell; ++i) {
      unsigned char_end = p - start;
      FQ_VERIFY(cells_[i] == char_end);
    }

    last_cell = cur_cell;
  }

  FQ_VERIFY(cur_cell == getWidth());
  FQ_VERIFY(cells_.back() == chars_.size());
}

void FQTermTextLine::verifyCellRange(unsigned cell_begin, unsigned cell_end) const {
  FQ_VERIFY(cell_begin <= cell_end);
  FQ_VERIFY(cell_end <= getWidth());

  unsigned real_cell_begin = getCellBegin(cell_begin);
  FQ_VERIFY(real_cell_begin == cell_begin);

  unsigned real_cell_end = getCellBegin(cell_end);
  FQ_VERIFY(real_cell_end == cell_end);

  unsigned char_begin = getCharBegin(cell_begin);
  FQ_VERIFY(char_begin <= chars_.size());

  unsigned char_end = getCharEnd(cell_end);
  FQ_VERIFY(char_begin <= char_end);
  FQ_VERIFY(char_end <= chars_.size());

}

#endif

FQTermTextLine::FQTermTextLine(unsigned max_cell_count)
    : max_cell_count_(max_cell_count),
      dirty_cell_begin_(0),
      dirty_cell_end_(max_cell_count),
      stored_color_(0),
      stored_attr_(0){
  cells_.reserve(max_cell_count_);
  cell_colors_.reserve(max_cell_count_);
  cell_attrs_.reserve(max_cell_count_);
  chars_.reserve(max_cell_count_);
}

FQTermTextLine::~FQTermTextLine() {
}

void FQTermTextLine::setMaxCellCount(unsigned max_cell_count) {
  // FIXME: how about dirty flag?
  max_cell_count_ = max_cell_count;

  if (cells_.size() > max_cell_count_) {
    // TODO_UTF16: what if an incomplete utf-16 byte left. 
    cells_.resize(max_cell_count_);
    cell_colors_.resize(max_cell_count_);
    cell_attrs_.resize(max_cell_count_);
    chars_.resize(cells_.back());
  }
}

// Whether this line of text contains blink characters.
bool FQTermTextLine::hasBlink() const {
  bool blink = false;

  char tempea;
  for (unsigned i = 0; i < cells_.size(); i++) {
    tempea = cell_attrs_[i];
    if (GETBLINK(tempea)) {
      blink = true;
      break;
    }
  }

  return blink;
}

void FQTermTextLine::setDirtyFlag(unsigned cell_begin, unsigned cell_end) {
#ifdef TEXT_LINE_CHECK
  FQ_VERIFY(cell_begin <= cell_end);
  FQ_VERIFY(cell_end <= max_cell_count_);
#endif
  if (dirty_cell_begin_ > cell_begin) {
    dirty_cell_begin_ =  cell_begin;
  }

  if (dirty_cell_end_ < cell_end) {
    dirty_cell_end_ = cell_end;
  }
}

void FQTermTextLine::safelySetDirtyFlag(unsigned cell_begin, unsigned cell_end) {
  if (cell_end > max_cell_count_) cell_end = max_cell_count_;

  if (cell_begin >= cell_end) 
    return;

  setDirtyFlag(cell_begin, cell_end);
}

void FQTermTextLine::clearDirtyFlag() {
  dirty_cell_begin_ = 0;
  dirty_cell_end_ = 0;
}

bool FQTermTextLine::getDirtyCells(unsigned &cell_begin, unsigned &cell_end) const {
  cell_begin = dirty_cell_begin_;
  cell_end = dirty_cell_end_;
  return dirty_cell_begin_ < dirty_cell_end_;
}

void FQTermTextLine::getPlainText(unsigned cell_begin, unsigned cell_end,
                                 QString &result) const {
  verifyCellRange(cell_begin, cell_end);

  if (cells_.size() == 0 || cell_begin == cell_end) return;

  unsigned char_begin = getCharBegin(cell_begin);
  unsigned char_end = getCharEnd(cell_end);

  result.insert(result.size(),
                (QChar *) &chars_[char_begin],
                char_end - char_begin);

  typedef char Verify_QChar_Is_Two_Bytes[
    (sizeof(QChar) == 2)? 1 : -1];
}

void FQTermTextLine::getAnsiText(unsigned cell_begin, unsigned cell_end,
                                QString &result,
                                const char *escape) const {
  verifyCellRange(cell_begin, cell_end);

  if (cells_.size() == 0 || cell_begin == cell_end) return;

  unsigned cur_cell = cell_begin;
  unsigned next_cell = cell_begin;
  
  while (cur_cell < cell_end) {
    unsigned char color = cell_colors_[cur_cell];
    unsigned char attr = cell_attrs_[cur_cell];
    
    for (next_cell = cur_cell;
         next_cell < cell_end; 
         ++next_cell) {
           if (!(cell_colors_[next_cell] == color
             && cell_attrs_[next_cell] == attr)) {
               if (next_cell > getCellBegin(next_cell)) {
                 continue;
               } else {
                 break;
               } 
           }
           
    }

    getAnsiCtrlSeq(color, attr, escape, result); 
    getPlainText(cur_cell, next_cell, result);
    result += escape;
    result += "0m";
    
    cur_cell = next_cell;
  }
}


void FQTermTextLine::appendWhiteSpace(
    unsigned count, unsigned char color, unsigned char attr, UTF16 space) {
  FQ_ASSERT(get_str_width(space) == 1);

  cells_.reserve(getWidth() + count);
  max_cell_count_ = qMax(max_cell_count_, getWidth() + count);

  cell_colors_.reserve(getWidth() + count);
  cell_attrs_.reserve(getWidth() + count);

  setDirtyFlag(getWidth(), getWidth() + count);

  chars_.insert(chars_.end(), count, space);
  cell_colors_.insert(cell_colors_.end(), count, color);
  cell_attrs_.insert(cell_attrs_.end(), count, attr);

  int c = getCharEnd(getWidth());
  for (unsigned i = 0; i < count; ++i) {
    cells_.push_back(c + i + 1);
  }

  verifyTextLine();
}

void FQTermTextLine::insertWhiteSpace(unsigned count, unsigned cell_begin,
                                     unsigned char color, unsigned char attr,
                                     UTF16 space) {
  FQ_ASSERT(get_str_width(space) == 1);

  breakCell(cell_begin);

  verifyCellRange(cell_begin, getWidth());

  unsigned char_begin = getCharBegin(cell_begin);

  max_cell_count_ = qMax(max_cell_count_, getWidth() + count);

  chars_.insert(chars_.begin() + char_begin, count, space);
  cell_colors_.insert(cell_colors_.begin() + cell_begin, count, color);
  cell_attrs_.insert(cell_attrs_.begin() + cell_begin, count, attr);

  cells_.insert(cells_.begin() + cell_begin, count, 0);
  for (unsigned i = cell_begin; i < cell_begin + count; ++i) {
    cells_[i] = char_begin + (i - cell_begin) + 1;
  }
  for (unsigned i = cell_begin + count; i < getWidth(); ++i) {
    cells_[i] += count;
  }

  setDirtyFlag(cell_begin, this->getWidth());

  verifyTextLine();
}

void FQTermTextLine::replaceWithWhiteSpace(
    unsigned count, unsigned cell_begin, unsigned cell_end,
    unsigned char color, unsigned char attr, UTF16 space) {
  if(cell_end>cells_.size() || cell_begin>=cell_end)
    return;
  deleteText(cell_begin, cell_end);
  insertWhiteSpace(count, cell_begin, color, attr, space);
}



void FQTermTextLine::insertText(const UTF16 *str, unsigned count,
                               unsigned cell_begin,
                               unsigned char color, unsigned char attr, int charstate) {
  if (count == 0) return;

  // Calculate how many cells should be occupied for str.
  // TODO: avoid compute the width of str twice.
  int width = get_str_width(str, count);

  if (width == 0) {
#ifdef TEXT_LINE_CHECK
    //FQ_VERIFY(false);
    //TODO: enable utf8 decoding
#endif
    return;
  }

  if (charstate & FQTermTextLine::FIRSTPART) {
    stored_color_ = color;
    stored_attr_ = attr;
    width--;
    count--;
    if (width == 0 || count == 0) {
      return;
    }
  }

  breakCell(cell_begin);

  verifyCellRange(cell_begin, getWidth());

  unsigned char_begin = getCharBegin(cell_begin);

  chars_.insert(chars_.begin() + char_begin, str, str + count);

  // Insert colors.
  cell_colors_.insert(cell_colors_.begin() + cell_begin, width, color);

  // Insert attrs.
  cell_attrs_.insert(cell_attrs_.begin() + cell_begin, width, attr);

  if (charstate & FQTermTextLine::SECONDPART){
    cell_colors_[cell_begin] = stored_color_;
    cell_attrs_[cell_begin] = stored_attr_;
  }
  

  // Change the cells_ mapping to chars_.
  cells_.insert(cells_.begin() + cell_begin, width, 0);
  max_cell_count_ = qMax(max_cell_count_, getWidth());

  unsigned last_cell = cell_begin;
  unsigned cur_cell = last_cell;
  for (const UTF16 *p = str; p < str +count;) {
    int w = mk_advace_at_least_one_cell(&p, str + count);

#ifdef TEXT_LINE_CHECK
    // TODO_UTF16: FIXME: deal with decoding error.
    FQ_VERIFY(w >= 0);
#endif

    cur_cell += w;

    for (unsigned i = last_cell; i < cur_cell; ++i) {
      cells_[i] = char_begin + (p - str);
    }

    last_cell = cur_cell;
  }

#ifdef TEXT_LINE_CHECK
  FQ_VERIFY(cur_cell == cell_begin + width);
#endif

  for (unsigned i = cell_begin + width; i < cells_.size(); ++i) {
    cells_[i] += count;
  }

  

  setDirtyFlag(cell_begin, cells_.size());

  verifyTextLine();
}

void FQTermTextLine::deleteText(unsigned cell_begin, unsigned cell_end) {
  if (cells_.size() == 0 || cell_begin == cell_end) return;

  breakCell(cell_begin);
  breakCell(cell_end);

  verifyCellRange(cell_begin, cell_end);

  setDirtyFlag(cell_begin, cells_.size());

  unsigned char_begin = getCharBegin(cell_begin);
  unsigned char_end = getCharEnd(cell_end);

  chars_.erase(chars_.begin() + char_begin, chars_.begin() + char_end);

  cells_.erase(cells_.begin() + cell_begin, cells_.begin() + cell_end);
  cell_colors_.erase(cell_colors_.begin() + cell_begin,
                     cell_colors_.begin() + cell_end);
  cell_attrs_.erase(cell_attrs_.begin() + cell_begin,
                    cell_attrs_.begin() + cell_end);

  for (unsigned i = cell_begin; i < cells_.size(); ++i) {
    cells_[i] -= (char_end - char_begin);
  }

  verifyTextLine();
}

void FQTermTextLine::deleteAllText() {
  deleteText(0, getWidth());
}

void FQTermTextLine::replaceText(const UTF16 *str, unsigned count,
                                unsigned cell_begin, unsigned cell_end,
                                unsigned char color, unsigned char attr, int charstate) {
  if (charstate & FQTermTextLine::FIRSTPART) {
    stored_color_ = color;
    stored_attr_ = attr;
    count--;
    if (count == 0)
    {
      return;
    }
    charstate &= ~FQTermTextLine::FIRSTPART;
  }

  deleteText(cell_begin, cell_end);
  insertText(str, count, cell_begin, color, attr, charstate);

  setDirtyFlag(cell_begin, cells_.size());
}


unsigned FQTermTextLine::getCellBegin(unsigned cell_begin) const {
#ifdef TEXT_LINE_CHECK
  FQ_VERIFY(cell_begin <= getWidth());
#endif

  if (cell_begin == cells_.size()) return cell_begin;

  while(cell_begin > 0 && cells_[cell_begin - 1] == cells_[cell_begin]) {
    --cell_begin;
  }

  return cell_begin;
}

unsigned FQTermTextLine::getCellEnd(unsigned cell_end) const {
#ifdef TEXT_LINE_CHECK
  FQ_VERIFY(cell_end <= getWidth());
#endif

  if (cell_end == 0) return cell_end;

  while (cell_end < cells_.size() && cells_[cell_end - 1] == cells_[cell_end]) {
    ++cell_end;
  }

  return cell_end;
}

unsigned FQTermTextLine::getCharBegin(unsigned cell_begin) const {
#ifdef TEXT_LINE_CHECK
  FQ_VERIFY(cell_begin == getCellBegin(cell_begin));
#endif

  if (cell_begin == 0) return 0;

  return cells_[cell_begin - 1];
}

unsigned FQTermTextLine::getCharEnd(unsigned cell_end) const {
#ifdef TEXT_LINE_CHECK
  FQ_VERIFY(cell_end == getCellEnd(cell_end));
#endif

  if (cell_end == 0) return 0;

  return cells_[cell_end - 1];
}

bool FQTermTextLine::breakCell(unsigned cell_begin) {
  unsigned real_cell_begin = getCellBegin(cell_begin);
  if (real_cell_begin == cell_begin) return false;

  unsigned real_cell_end = getCellEnd(cell_begin);

  verifyCellRange(real_cell_begin, real_cell_end);

  unsigned cell_count = real_cell_end - real_cell_begin;

  replaceWithWhiteSpace(cell_count, real_cell_begin, real_cell_end,
                        cell_colors_[cell_begin], cell_attrs_[cell_begin], URC);

  return true;
}

void FQTermTextLine::getAnsiCtrlSeq(
    unsigned char color, unsigned char attr, 
    const char *escape, QString &result) const {

    int fg = GETFG(color) + 30;
    int bg = GETBG(color) + 40;

    result += escape;
    result += QString("%1;%2").arg(fg).arg(bg);

    if (GETBRIGHT(attr)) {
      result += ";1";
    }
    if (GETDIM(attr)) {
      result += ";2";
    }
    if (GETUNDERLINE(attr)) {
      result += ";4";
    }
    if (GETBLINK(attr)) {
      result += ";5";
    }
    if (GETRAPIDBLINK(attr)) {
      result += ";6";
    }
    if (GETREVERSE(attr)) {
      result += ";7";
    }
    if (GETINVISIBLE(attr)) {
      result += ";8";
    }

    result += "m";
}


}  // namespace FQTerm
