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

#ifndef FQTERM_TEXTLINE_H
#define FQTERM_TEXTLINE_H

#include <vector>

#include "fqwcwidth.h"
#include "fqterm.h"
class QString;

namespace FQTerm {
/*
  A text line consists of several characters in a sequence of cells. A
  character may occupy some cells, e.g. each English letter occupies
  one cell, while a CJK character needs two.

  This class is used to manipolate a text line, delete/replace
  characters in a given cell range, insert characters before a cell,
  and retrieve characters/colors/attributs of a gaven cell range.
*/
class FQTermTextLine {
 public:
  FQTermTextLine(unsigned max_cell_count);
  ~FQTermTextLine();

  enum CHARSTATE{NORMAL = 0x00, FIRSTPART = 0x01, SECONDPART = 0x02};
  // Get the number of used cells.
  unsigned getWidth() const {return cells_.size();}

  void setMaxCellCount(unsigned max_cell_count);
  unsigned getMaxCellCount() const { return max_cell_count_; }
  
  // Whether this line of text contains blink characters.
  bool hasBlink() const;

  // Set/get the dirty flag, that indicates which cells have been
  // modified.
  void setDirtyFlag(unsigned cell_begin, unsigned cell_end);
  void safelySetDirtyFlag(unsigned cell_begin, unsigned cell_end);
  bool getDirtyCells(unsigned &cell_begin, unsigned &cell_end) const;
  void clearDirtyFlag();
  void setAllDirty() {setDirtyFlag(0, max_cell_count_);}

  const unsigned char *getColors() const {
    return cells_.size() == 0 ? NULL : &cell_colors_[0];
  }

  const unsigned char *getAttributes() const {
    return cells_.size() == 0 ? NULL : &cell_attrs_[0];
  }

  // Get plain text in [cell_begin, cell_end).
  void getPlainText(unsigned cell_begin, unsigned cell_end,
                    QString &result) const;

  void getAllPlainText(QString &result) const {
    getPlainText(0, getWidth(), result);
  }
  
  // Get text in cells [cell_begin, cell_end), with color and
  // attribute in ANSI format.
  void getAnsiText(unsigned cell_begin, unsigned cell_end,
                   QString &result,
                   const char *escape = "\x1b\x1b") const;

  void getAllAnsiText(QString &result, const char* escape = "\x1b\x1b") const {
    getAnsiText(0, getWidth(), result, escape);
  }

  void appendWhiteSpace(unsigned count,
                        unsigned char color = NO_COLOR,
                        unsigned char attr = NO_ATTR,
                        UTF16 space = ' ');

  void insertWhiteSpace(unsigned count, unsigned cell_begin,
                        unsigned char color = NO_COLOR,
                        unsigned char attr = NO_ATTR,
                        UTF16 space = ' ');

  void replaceWithWhiteSpace(unsigned count,
                             unsigned cell_begin, unsigned cell_end,
                             unsigned char color = NO_COLOR,
                             unsigned char attr = NO_ATTR,
                             UTF16 space = ' ');
  
  // insert string at specified cell,
  void insertText(const UTF16 *str, unsigned count,
                  unsigned cell_begin,
                  unsigned char color, unsigned char attr, int charstate = FQTermTextLine::NORMAL);
  void replaceText(const UTF16 *str, unsigned count,
                   unsigned cell_begin, unsigned cell_end,
                   unsigned char color, unsigned char attr, int charstate = FQTermTextLine::NORMAL);
  void deleteText(unsigned cell_begin, unsigned cell_end);
  void deleteAllText();

  // If cell_begin is a begining of a readable character, return
  // cell_begin itself, otherwise return the previous cell which starts a
  // readable character.
  unsigned getCellBegin(unsigned cell_begin) const;

  // If cell_end is a ending of a readable character, return cell_end
  // itself, otherwise return the next cell which finishes a
  // readable character.
  unsigned getCellEnd(unsigned cell_end) const;

  unsigned getCharBegin(unsigned cell_begin) const;
  unsigned getCharEnd(unsigned cell_end) const;

 private:
  unsigned max_cell_count_; // max number of cells allowed.
  
  std::vector<uint16_t> cells_;  // store the index of character in this->chars_ for each cell.
  std::vector<unsigned char> cell_colors_; // store the color for each cell.
  std::vector<unsigned char> cell_attrs_; // store the attribute for each cell.
  std::vector<UTF16> chars_;  // store all the character.

  unsigned char stored_color_;
  unsigned char stored_attr_;

  // range of changed text.
  unsigned dirty_cell_begin_, dirty_cell_end_;

  // if the given cell isn't a beginning of characters, 
  // i.e is in the middle of a wide character,
  // break the character into several unicode replacement characters (URC)
  // to make the given cell be a beginning of character.
  bool breakCell(unsigned cell_begin);

  // calculate ANSI escape sequence with given color, attribute and escape strings.
  void getAnsiCtrlSeq(unsigned char color, unsigned char attr,
                      const char *escape, QString &result) const;

  // Two functions for debugging/testing.
  void verifyTextLine() const;
  void verifyCellRange(unsigned cell_begin, unsigned cell_end) const;
};

}  // namespace FQTerm

#endif  // FQTERM_TEXTLINE_H
