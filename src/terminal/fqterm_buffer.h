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

#ifndef FQTERM_BUFFER_H
#define FQTERM_BUFFER_H

#include <QList>
#include <QObject>
#include <QPoint>

#include "fqterm_text_line.h"
class QString;
class QByteArray;
class QRect;

namespace FQTerm {

class FQTermTextLine;

/* Text buffer consists of characters in a matrix of cells.
 * 
 *                 Buffer
 *     +--------------------------------+
 *     ||---->                          |
 *     ||    x                          |
 *     ||                               |
 *     |V Y                             |
 *     |                                |
 *     .                                .
 *     .                                .
 *     .        histroical lines        .
 *     .                                .
 *     .                                .
 *     |                                |
 *     |                                |
 *     |                                |
 *     |                                |
 *     |+------------------------------+|
 *     |||---->                        ||
 *     |||    x                        ||
 *     |||          terminal           ||
 *     ||V Y                           ||
 *     ||                              ||
 *     ||                              ||
 *     ||         | (caret)            ||
 *     ||                              ||
 *     ||                              ||
 *     |+------------------------------+|
 *     +--------------------------------+
 *
 * Text buffer, to manage lines of characters with colors and
 * attributes, caret positions, terminal size/margins, selection
 * region, historical text lines, buffer write mode (insert or
 * replace), and etc.
 *
 * Note: There are two coordinates, relative to terminal or entire
 * buffer. When we use (column, row) it indicates terminal coordinate,
 * while when we use (column, line) it indicates buffer coordinate.
 * This rule applies both in variable names and function names.
 **/
class FQTermBuffer: public QObject {
  Q_OBJECT;
 public:
  enum TermMode {
    INSERT_MODE,     /* otherwise replace mode */
    ANSI_MODE,       /* otherwise VT52 mode */
    SMOOTH_MODE,     /* otherwse jump mode */
    NEWLINE_MODE,    /* otherwise line feed mode */
    CURSOR_MODE,     /* otherwise reset cursor mode */
    NUMERIC_MODE,    /* otherwise application mode */
    ORIGIN_MODE,     /* otherwise absolute mode */
    AUTOWRAP_MODE,   /* otherwise non-autowrap mode */
    AUTOREPEAT_MODE, /* otherwise non-autorepeat mode */
    LIGHTBG_MODE,    /* otherwise dark background mode */
  };

  enum VtCharSet {
    UNITED_KINGDOM_SET,
    ASCII_SET,
    SPECIAL_GRAPHICS,
    ALTERNATE_CHARACTER_ROM_STANDARD_CHARACTER_SET,
    ALTERNATE_CHARACTER_ROM_SPECIAL_GRAPHICS
  };
  
  FQTermBuffer(int column, int row, int max_hist_line, bool is_bbs);
  ~FQTermBuffer();

  // Get text lines.
  // Return null if the line_index is out of bound, return NULL.
  const FQTermTextLine *getTextLineInBuffer(int line_index) const;
  const FQTermTextLine *getTextLineInTerm(int line_index) const;

  // Get number of columns and rows of the term, or lines of the
  // entire buffer.
  int getNumColumns() const;
  int getNumRows() const;
  int getNumLines() const;
  
  // Set the size of the screen.
  void setTermSize(int col, int row);
  // set margins in term. (Generally speaking, most operations are
  // restrictied in [top_row_, bottom_row_] of terminal.)
  void setMargins(int top_row, int bottom_row);

  // reset terminal
  void termReset();
  
  // the caret's coordinate in term or buffer.
  int getCaretColumn() const;
  int getCaretRow() const;
  int getCaretLine() const;

  // Set current attribute.
  void setCurrentAttr(unsigned char color, unsigned char attr);

  // Below are a series of functions to modify the content of buffer.
  // These functions will work from the current caret position.
  // All the behaviours of these functions may be influenced by the
  // mode of this buffer (is_insert_mode_, is_newline_mode_, and
  // etc.).

  void writeText(const QString &cstr, int charstate = FQTermTextLine::NORMAL);

  void fillScreenWith(char c);

  void insertSpaces(int count);
  void insertLines(int count);

  void deleteText(int cell_count);
  void deleteLines(int line_count);

  void eraseText(int cell_count);

  void eraseToLineBegin();
  void eraseToLineEnd();
  void eraseEntireLine();

  void eraseToTermBegin();
  void eraseToTermEnd();
  void eraseEntireTerm();

  // Set a line of buffer to have been changed from start to end.
  void setLineChanged(int index, int cell_begin, int cell_end);
  void clearLineChanged(int index);
  void setLineAllChanged(int index);

  // Functions about caret.
  // If the caret is moved, the cells of the position of the last caret will
  // be marked as changed.
  void moveCaretOffset(int coloumn_offset, int row_offset,
                       bool scroll_if_necessary = false);
  void moveCaretTo(int coloumn, int row, bool scroll_if_necessary = false);
  void changeCaretPosition(int coloumn, int row);
  void saveCaret();
  void restoreCaret();

  void SelectVtCharacterSet(VtCharSet charset, bool G0);
  void invokeCharset(bool G0);

  // non-printing characters
  void tab();
  void carriageReturn();
  void lineFeed();

  void addTabStop();
  void clearTabStop(bool clear_all);
  
  // Select Mode (SM) and Reset Mode (RM) functions.
  // See ANSI X3.64 Mode-Changing Parameters.
  void setMode(TermMode);
  void resetMode(TermMode);

  bool isCursorMode() const { return is_cursor_mode_; }
  bool isAnsiMode() const { return is_ansi_mode_; }
  bool isNumericMode() const { return is_numeric_mode_; }
  bool isAutoRepeatMode() const { return is_autorepeat_mode_; }
  bool isLightBackgroundMode() const { return is_lightbg_mode_; }
  bool isNewLineMode() const {return is_newline_mode_;}
  
  // for test
  void startDecode();
  void endDecode();

  // Functions about selection in buffer.
  void setSelect(const QPoint &start_point, const QPoint &end_point);
  void clearSelect();
  bool isSelected(const QPoint &cell, bool isRectSel) const;
  bool isSelected(int line_index) const;

  QString getTextSelected(bool is_rect_sel, bool is_color_copy,
                          const QByteArray &escape) const;

  // Get the rectangle of selected text of a certain line in buffer.
  // Note: please ensure there exits any cell of the line line_index
  // is selected.
  QRect getSelectRect(int line_index, bool is_rect_sel) const;

  //Scroll line && adjust caret pos.
  //Scroll must occur at the line where caret is
  void scrollTerm(int numRows);
  
 signals:
  void bufferSizeChanged();
  void termSizeChanged(int column, int row);
  void onSetTermSize(int col, int row);
  void caretChangeRow();
 private:
  // Scroll lines between startRow and bottom_row_ (see setMargin()).
  // num > 0 scroll up.
  // num < 0 scroll down.
  void scrollLinesInTerm(int startRow, int numRows);

  // Replace a area of cells with spaces. 
  void clearArea(int startColumn, int startRow, 
                 int width, int height,
                 unsigned char color, unsigned char attr);

  // Append n empty lines to the buffer and make the first n lines of
  // term historical.
  void addHistoryLine(int n);

  int getTabStop(int column);

  struct Caret {
    int column_, row_;
    int color_, attr_;
    VtCharSet G0_, G1_;

    Caret();
  };

  // terminal size.
  int num_columns_, num_rows_;

  // term margins. (Generally speaking, most operations are
  // restrictied in [top_row_, bottom_row_] of terminal.)
  int top_row_, bottom_row_;

  // Historical data length and max length.
  int num_hist_lines_, max_num_hist_lines_;

  // All lines of text, including both historical lines and lines in
  // current terminal.
  //
  // Note: the number of historical lines, num_hist_lines, may
  // increase from 0 to max_num_hist_lines_.
  QList<FQTermTextLine *> text_lines_;

  void *tab_stops_;

  // The caret in terminal
  Caret caret_;
  Caret last_saved_caret_;

  bool is_g0_used_;  // is G0 or G1 charset used;

  bool is_insert_mode_;     // Is insert or replace mode.
  bool is_ansi_mode_;       // Is ansi or vt52 mode.
  bool is_smoothscroll_mode_; // Is smooth or jump scrolling mode.
  bool is_newline_mode_;    // Is newline or linefeed mode.
  bool is_cursor_mode_;     // Is cursor key mode.
  bool is_numeric_mode_;    // Is numeric or application mode.
  bool is_origin_mode_;     // Is origin or absolute mode.
  bool is_autowrap_mode_;   // Is auto-wrap mode enabled.
  bool is_autorepeat_mode_; // Is auto-repeat mode enabled.
  bool is_lightbg_mode_;    // Is ligth background or dark backgrond mode.

  // Selection in buffer. If section start and end are in the same
  // line, start.x should be equal to or less than end.x. Otherwise
  // start.y should be less than end.y.
  //
  // If selection_start_ = selection_start_ = (-1, -1), the selection
  // is empty.
  QPoint selection_start_;
  QPoint selection_end_;

  // whether this buffer is used for a bbs session.
  bool is_bbs_;
}; 

}  // namespace FQTerm

#endif // FQTERM_BUFFER_H
