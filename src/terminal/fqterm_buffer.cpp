// SPDX-License-Identifier: GPL-2.0-or-later

#include <algorithm>
#include <map>
#include <vector>

#include <QString>
#include <QRect>
#include <QRegExp>

#include "fqterm.h"
#include "common.h"
#include "fqterm_buffer.h"
#include "fqterm_text_line.h"

namespace FQTerm {

typedef std::vector<int> TabStops;

FQTermBuffer::FQTermBuffer(int column, int row, int max_hist_line, bool is_bbs) {
  tab_stops_ = new TabStops;
  num_rows_ = row;
  num_columns_ = column;
  max_num_hist_lines_ = max_hist_line;
  is_bbs_ = is_bbs;

  num_hist_lines_ = 0;

  while (text_lines_.count() < num_rows_) {
    text_lines_.append(new FQTermTextLine(num_columns_));
  }

  top_row_ = 0;
  bottom_row_ = num_rows_ - 1;

  is_g0_used_ = true;  

  is_insert_mode_ = false;
  is_ansi_mode_ = true;
  is_newline_mode_ = false;
  is_cursor_mode_ = false;
  is_numeric_mode_ = true;
  is_origin_mode_ = false;
  is_autowrap_mode_ = false;
  is_autorepeat_mode_ = true;
  is_lightbg_mode_ = false;  

  selection_start_ = QPoint(-1, -1);
  selection_end_ = QPoint(-1, -1);
}

FQTermBuffer::~FQTermBuffer(){
  foreach(FQTermTextLine *line, text_lines_) {
    delete line;
  }

  delete (TabStops *)tab_stops_;  
}

FQTermBuffer::Caret::Caret() 
    : column_(0),
      row_(0),
      color_(NO_COLOR),
      attr_(NO_ATTR),
      G0_(ASCII_SET),
      G1_(ASCII_SET) {
}

void FQTermBuffer::setTermSize(int col, int row) {
  FQ_TRACE("term", 3) << "Change term size to "
                    << col << "x" << row;

  if (num_columns_ == col && num_rows_ == row) {
    return;
  }

  clearSelect();

  if (num_rows_ < row) {
    for (int i = 0; i < row - num_rows_; i++) {
      text_lines_.append(new FQTermTextLine(col));
    }
  } else if (num_rows_ > row) {
    for (int i = 0; i < num_rows_ - row; i++) {
      delete text_lines_.takeLast();
    }
  }

  num_columns_ = col;
  num_rows_ = row;

  top_row_ = 0;
  bottom_row_ = num_rows_ - 1;

  // TODO: how about saved caret column?
  // TODO: why make last saved equal to current position?
  last_saved_caret_.row_ = caret_.row_ = qMin(caret_.row_, row - 1);

  for (int i = 0; i < text_lines_.size(); ++i) {
    text_lines_.at(i)->setMaxCellCount(num_columns_);
  }

//  this->clearArea(0, 0, num_columns_, num_rows_, 0, 0);
  //  this->moveCaretTo(0, 0);

  moveCaretTo(qMin(caret_.column_, col - 1), qMin(caret_.row_, row - 1));
  
  emit onSetTermSize(col, row);
  emit termSizeChanged(num_columns_, num_rows_);
}

int FQTermBuffer::getNumColumns() const {
  return num_columns_;
}

int FQTermBuffer::getNumRows() const {
  return num_rows_;
}

int FQTermBuffer::getNumLines() const {
  return num_rows_ + num_hist_lines_;
}

const FQTermTextLine *FQTermBuffer::getTextLineInBuffer(int line_index) const {
  return text_lines_.value(line_index, NULL);
}

const FQTermTextLine *FQTermBuffer::getTextLineInTerm(int line_index) const {
  return text_lines_.value(line_index + num_hist_lines_, NULL);
}

void FQTermBuffer::setCurrentAttr(unsigned char color, unsigned char attr) {
  caret_.color_ = color;
  caret_.attr_ = attr;
}

void FQTermBuffer::writeText(const QString &str, int charstate) {
    
  QString cstr = str;

  FQ_TRACE("term", 8) << "Add text: \"" << cstr << "\"";

#if 0  
  if ((is_g0_used_ && caret_.G0_ == SPECIAL_GRAPHICS) ||
      (!is_g0_used_ && caret_.G1_ == SPECIAL_GRAPHICS)) {

    // FIXME: will this break the utf-16 sequence?
    for (int i = 0; i < cstr.size(); ++i) {
      if (cstr[i] >= 0137 && cstr[i] <= 0176) {
        cstr[i] = VT_SPECIAL_GRAPHICS_TABLE[cstr.at(i).unicode()];
      }
    }
  }
#endif  

  // Insert the str into the buffer. Different pieces of text might be
  // written into different lines.
  while (!cstr.isEmpty()) {
    FQTermTextLine *line = text_lines_.value(num_hist_lines_ + caret_.row_, NULL);

    if (line == NULL) { 
      FQ_TRACE("error", 0) << "Error: setBuffer null line";
      return ;
    }

    if (charstate & FQTermTextLine::SECONDPART) {
      moveCaretOffset(-1, 0);
    } 

    if (caret_.column_ >= (int)line->getMaxCellCount()) {
      // move the caret to the next line.
      moveCaretTo(0, caret_.row_ + 1, true);
      continue;
    }

    if (caret_.column_ > (int)line->getWidth()) {
      line->appendWhiteSpace(caret_.column_ - line->getWidth());
    }

    unsigned cell_begin = line->getCellBegin(caret_.column_);
    int max_width = line->getMaxCellCount() - cell_begin;
    int element_consumed = 0;
    int width = get_str_width((const UTF16 *)cstr.data(), cstr.size(),
                              max_width, element_consumed);

    if (width < 0) {
      break;
    }

    if (is_insert_mode_) {
      // FIXEME: How to move cursor if the entire line is wider than
      // line->getMaxCellCount() after insertion?
      line->insertText((UTF16 *)cstr.data(), element_consumed, cell_begin,
                       caret_.color_, caret_.attr_, charstate);
      if ((int)line->getWidth() > num_columns_) {
        line->deleteText(num_columns_, line->getWidth());
      }
    } else {
      line->replaceText((UTF16 *)cstr.data(), element_consumed, 
                        cell_begin, qMin(cell_begin + width, line->getWidth()),
                        caret_.color_, caret_.attr_, charstate);
    }
    moveCaretOffset(width, 0);

	if (element_consumed == 0)
	{
		element_consumed = 1;
	}

    if (element_consumed == cstr.size()) {
      break;
    }
	
    cstr.remove(0, element_consumed);
  }
}

void FQTermBuffer::lineFeed() {
  FQ_TRACE("term", 8) << "add a new line";
  if (caret_.row_ == bottom_row_)
    scrollLinesInTerm(top_row_, 1);
  if (is_newline_mode_) {
    moveCaretOffset(-caret_.column_, 1);
  } else {
    moveCaretOffset(0, 1);
  }
}

void FQTermBuffer::tab() {
  FQ_TRACE("term", 8) << "add a tab";  
  int x = getTabStop(caret_.column_);

  moveCaretTo(x, caret_.row_);
}

int FQTermBuffer::getTabStop(int column) {
  
  std::vector<int> &tabs = *(TabStops *)tab_stops_;

  if (tabs.size() == 0) {
    int res = ((caret_.column_) / 8 + 1) * 8;
    return (res < num_columns_) ? res : num_columns_ - 1;
  }

  std::vector<int>::iterator it =
      std::upper_bound(tabs.begin(), tabs.end(), column);

  int res = num_columns_ - 1;
  
  if (it != tabs.end() && *it < res) {
    res = *it;
  }

  if (res > num_columns_ - 1) {
    res = num_columns_ - 1;
  }

  return res;
}

void FQTermBuffer::addTabStop() {
  // FIXME: what if the caret is located beyond the term temporarily.
  int x = caret_.column_;
  
  std::vector<int> &tabs = *(TabStops *)tab_stops_;

  std::vector<int>::iterator it =
      std::lower_bound(tabs.begin(), tabs.end(), x);
  
  if (it == tabs.end() || *it != x) {
    tabs.insert(it, x);
  }
}

void FQTermBuffer::clearTabStop(bool clear_all) {
  // FIXME: what if the caret is located beyond the term temporarily.
  std::vector<int> &tabs = *(TabStops *)tab_stops_;
  
  if (clear_all) {
    tabs.clear();
    return;
  }

  int x = caret_.column_;

  std::vector<int>::iterator it =
      std::lower_bound(tabs.begin(), tabs.end(), x);

  if (it != tabs.end() && *it == x) {
    tabs.erase(it);
  }
}

void FQTermBuffer::setMargins(int top, int bottom) {
  FQ_TRACE("term", 3) << "Set margins: [" << top << ", " << bottom << "]";
      
  top_row_ = qMax(top, 0);
  bottom_row_ = qMin(qMax(bottom, 0), num_rows_ - 1);

  if (is_origin_mode_) {
    moveCaretTo(0, top_row_);
  } else {
    moveCaretTo(0, 0);    
  }
}

    // termReset(): deal with ANSI sequence <ESC> c
    // initialize the terminal property
    void FQTermBuffer::termReset()
    {
        FQ_TRACE("term", 3) << "Resetting terminal (<ESC> c)";
        top_row_ = 0;
        bottom_row_ = num_rows_ - 1;

        is_g0_used_ = true;
        is_insert_mode_ = false;
        is_ansi_mode_ = true;
        is_newline_mode_ = false;
        is_cursor_mode_ = false;
        is_numeric_mode_ = true;
        is_origin_mode_ = false;
        is_autowrap_mode_ = false;
        is_autorepeat_mode_ = true;
        is_lightbg_mode_ = false;
    }
    
void FQTermBuffer::moveCaretTo(int column, int row, bool scroll_if_necessary) {
  if (row != caret_.row_)
    emit caretChangeRow();

  FQ_TRACE("term", 5) << "Move caret to (" << column << ", " << row << ")";
  
  //If th

  // detect index boundary
  if (column >= num_columns_) {
    column = num_columns_;
  }
  if (column < 0) {
    column = 0;
  }

  int scroll_lines = 0;
 
  int stop = is_origin_mode_ ? top_row_ : 0;
  if (row < stop) {
    scroll_lines = row - stop;
    row = stop;
  }

  stop = is_origin_mode_ ? bottom_row_ : num_rows_ - 1;
  if (row > stop) {
    scroll_lines = row - stop;
    row = stop;
  }

  // Set dirty flag for cells of the last caret position.
  FQTermTextLine *line = text_lines_.value(num_hist_lines_ + caret_.row_, NULL);
  if (0 <= caret_.column_ && caret_.column_ < (int)line->getWidth()) {
    unsigned cell_begin = line->getCellBegin(caret_.column_);
    unsigned cell_end = line->getCellBegin(caret_.column_ + 1);
    line->setDirtyFlag(cell_begin, cell_end);
  }

  if (scroll_if_necessary) {
    scrollLinesInTerm(top_row_, scroll_lines);
  }

  caret_.column_ = column;
  caret_.row_ = row;
}

void FQTermBuffer::moveCaretOffset(int column_offset, int row_offset, bool scroll_if_necessary) {
  if (caret_.column_ >= num_columns_) {
    // it's only allowed that the caret_.column_ >= num_columns_
    // temporarily when a sequence of normal text is received. if we
    // found caret_.column_ is out of bounds in other case, we should
    // correct it first.
    if (is_bbs_) {
      // but the BBS (mysmth.net) assumes that the caret could be
      // located out of the screen :(
    } else {
      caret_.column_ = num_columns_ - 1;
    }
  }

  if (caret_.column_ + column_offset < 0) {
    column_offset = -caret_.column_;
  }

  moveCaretTo(caret_.column_ + column_offset, caret_.row_ + row_offset,
              scroll_if_necessary);
}

void FQTermBuffer::changeCaretPosition(int column, int row) {
  if (is_origin_mode_) {
    moveCaretTo(column, row + top_row_);
  } else {
    moveCaretTo(column, row);
  }
}

void FQTermBuffer::saveCaret() {
  FQ_TRACE("term", 5) << "save the caret.";  
  last_saved_caret_ = caret_;
}

void FQTermBuffer::restoreCaret() {
  FQ_TRACE("term", 5) << "restore the caret.";
  moveCaretTo(last_saved_caret_.column_, last_saved_caret_.row_);
  caret_ = last_saved_caret_; 
}

void FQTermBuffer::SelectVtCharacterSet(VtCharSet charset, bool G0) {
  if (G0) {
    caret_.G0_ = charset;
  } else {
    caret_.G1_ = charset;    
  }
}

void FQTermBuffer::invokeCharset(bool G0) {
  if (G0) {
    is_g0_used_ = true;
  } else {
    is_g0_used_ = false;
  }
}

void FQTermBuffer::carriageReturn() {
  FQ_TRACE("term", 8) << "carrige return";  
  moveCaretOffset(-caret_.column_, 0);
}

int FQTermBuffer::getCaretColumn() const {
  return caret_.column_;
}

int FQTermBuffer::getCaretRow() const {
  return caret_.row_;
}

int FQTermBuffer::getCaretLine() const {
  return caret_.row_ + num_hist_lines_;
}

// erase functions
void FQTermBuffer::eraseText(int cell_count) {
  FQ_TRACE("term", 8) << "erase " << cell_count << " cell(s) of text";
      
  const FQTermTextLine *line = text_lines_.at(caret_.row_ + num_hist_lines_);

  int x = line->getWidth() - caret_.column_;

  clearArea(caret_.column_, caret_.row_,
            qMin(cell_count, x), 1,
            caret_.color_, caret_.attr_);
}

void FQTermBuffer::deleteText(int cell_count) {
  FQ_TRACE("term", 8) << "delete " << cell_count << " cell(s) of text";
      
  FQTermTextLine *line = text_lines_.at(caret_.row_ + num_hist_lines_);

  int x = line->getWidth() - caret_.column_;

  if (cell_count >= x) {
    line->deleteText(caret_.column_, line->getWidth());
  } else {
    line->deleteText(caret_.column_, caret_.column_ + cell_count);
  }
}

void FQTermBuffer::fillScreenWith(char c) {
  FQ_TRACE("term", 5) << "fill screen with '" << c << "'";   
  for (int i = 0; i < num_rows_; i++) {
    FQTermTextLine *line = text_lines_[i + num_hist_lines_];
    line->deleteAllText();
    line->appendWhiteSpace(num_columns_, NO_COLOR, NO_ATTR, c);
  }
}

void FQTermBuffer::insertSpaces(int count) {
  FQ_TRACE("term", 8) << "insert " << count << " white space(s)";
  
  FQTermTextLine *line = text_lines_.at(caret_.row_ + num_hist_lines_);

  int x = line->getWidth() - caret_.column_;

  if (count >= x) {
    clearArea(caret_.column_, caret_.row_, x, caret_.row_,
              caret_.color_, caret_.attr_);
  } else {
    line->insertWhiteSpace(count, caret_.column_, caret_.color_, caret_.attr_);
  }
}

void FQTermBuffer::deleteLines(int line_count) {
  FQ_TRACE("term", 8) << "delete " << line_count << " line(s)";
  
  int y = bottom_row_ - caret_.row_;

  if (line_count >= y) {
    clearArea(0, caret_.row_, -1, y, caret_.color_, caret_.attr_);
  } else {
    scrollLinesInTerm(caret_.row_, line_count);
  }
}

void FQTermBuffer::insertLines(int count) {
  FQ_TRACE("term", 8) << "insert " << count << " line(s)";
  
  int y = bottom_row_ - caret_.row_;

  if (count >= y) {
    clearArea(0, caret_.row_, -1, y, caret_.color_, caret_.attr_);
  } else {
    scrollLinesInTerm(caret_.row_, -count);
  }
}

void FQTermBuffer::eraseToLineEnd() {
  FQ_TRACE("term", 8) << "erase to line end";
      
  clearArea(caret_.column_, caret_.row_, -1, 1, caret_.color_, caret_.attr_);
}

void FQTermBuffer::eraseToLineBegin() {
  FQ_TRACE("term", 8) << "erase to line begin";  
  clearArea(0, caret_.row_, caret_.column_ + 1, 1, caret_.color_, caret_.attr_);
}

void FQTermBuffer::eraseEntireLine() {
  FQ_TRACE("term", 8) << "erase entire line";  
  clearArea(0, caret_.row_, -1, 1, caret_.color_, caret_.attr_);
}

// Set a line of text on screen to have been changed from start to end.
void FQTermBuffer::setLineChanged(int index, int cell_begin, int cell_end) {
  FQ_ASSERT(0 <= index && index < num_rows_ + num_hist_lines_);
  text_lines_[index]->setDirtyFlag(cell_begin, cell_end);
}

void FQTermBuffer::clearLineChanged(int index) {
  FQ_ASSERT(0 <= index && index < num_rows_ + num_hist_lines_);
  text_lines_[index]->clearDirtyFlag();
}

void FQTermBuffer::setLineAllChanged(int index) {
  FQ_ASSERT(0 <= index && index < num_rows_ + num_hist_lines_);
  text_lines_[index]->setAllDirty();
}

void FQTermBuffer::scrollLinesInTerm(int startRow, int numRows) {
  if (numRows == 0 || startRow > bottom_row_) {
    return ;
  }
  if (startRow < top_row_) {
    startRow = top_row_;
  }

  // TODO: performance issue here. Reuse the old text lines.
  if (numRows > 0) {
    //We are scrolling the whole screen.
    if (startRow == 0 && caret_.row_ == num_rows_ - 1) {
      addHistoryLine(numRows);
    } else {
      // delete lines from startRow, insert lines on the bottom_row_.
      while (numRows) {
        delete text_lines_.takeAt(num_hist_lines_ + startRow);
        text_lines_.insert(num_hist_lines_ + bottom_row_, new FQTermTextLine(num_columns_));
        numRows--;
      }
    }
  }

  // TODO: performance issue here. Reuse the old text lines.
  if (numRows < 0) {
    // delete lines from bottom_row_, insert lines in the startRow.    
    while (numRows) {
      delete text_lines_.takeAt(num_hist_lines_ + bottom_row_);
      text_lines_.insert(num_hist_lines_ + startRow, new FQTermTextLine(num_columns_));
      numRows++;
    }
  }

  for (int i = num_hist_lines_ + startRow; i <= num_hist_lines_ + bottom_row_; i++) {
    text_lines_.at(i)->setAllDirty();
  }
}

void FQTermBuffer::eraseToTermEnd() {
  FQ_TRACE("term", 8) << "erase to term end";
  
  if (caret_.column_ == 0 && caret_.row_ == 0) {
    eraseEntireTerm();
    return ;
  }

  clearArea(caret_.column_, caret_.row_, -1, 1, caret_.color_, caret_.attr_);

  if (caret_.row_ < bottom_row_) {
    clearArea(0, caret_.row_ + 1, 
              -1, bottom_row_ - caret_.row_,
              caret_.color_, caret_.attr_);
  }
}

void FQTermBuffer::eraseToTermBegin() {
  FQ_TRACE("term", 8) << "erase to term begin";  
  if (caret_.column_ == num_columns_ - 1 && caret_.row_ == num_rows_ - 1) {
    eraseEntireTerm();
    return ;
  }

  clearArea(0, caret_.row_, caret_.column_ + 1, 1, caret_.color_, caret_.attr_);
  if (caret_.row_ > top_row_) {
    clearArea(0, top_row_, -1, caret_.row_, caret_.color_, caret_.attr_);
  }
}

void FQTermBuffer::eraseEntireTerm() {
  FQ_TRACE("term", 8) << "erase entire term";  
  addHistoryLine(num_rows_);
  clearArea(0, 0, num_columns_, num_rows_, caret_.color_, caret_.attr_);
}

// width = -1 : clear to end
void FQTermBuffer::clearArea(int startColumn, int startRow, 
                            int width, int height,
                            unsigned char color, unsigned char attr) {
  QByteArray cstr;

  FQTermTextLine *line;

  if (startRow + height > num_rows_) { 
    height = num_rows_ - startRow;
  }

  for (int i = startRow; i < height + startRow; i++) {
    line = text_lines_[i + num_hist_lines_];

    int w = width;
    
    if (startColumn < num_columns_) {
      if (w == -1) { 
        w = num_columns_ - startColumn;
      }

      int endX = startColumn + w;
      if (endX > (int)line->getWidth()) {
        endX = line->getWidth();
      }

      if(startColumn>=endX)
      {
          continue;
      }

      line->replaceWithWhiteSpace(w, startColumn, endX, color, attr);
    }
  }
}

void FQTermBuffer::addHistoryLine(int n) {
  bool is_full = (num_hist_lines_ == max_num_hist_lines_);

  // TODO: performance issue. Reuse the old lines.
  while (n) {
    if (num_hist_lines_ == max_num_hist_lines_) {
      delete text_lines_.takeFirst();
    }

    text_lines_.append(new FQTermTextLine(num_columns_));
    num_hist_lines_ = qMin(num_hist_lines_ + 1, max_num_hist_lines_);
    n--;
  }

  for (int i = num_hist_lines_ + 0; i < num_hist_lines_ + bottom_row_; i++) {
    text_lines_.at(i)->setAllDirty();
  }

  if (!is_full) {
    emit bufferSizeChanged();
  }
}

void FQTermBuffer::startDecode() {
  last_saved_caret_.column_ = caret_.column_;
  last_saved_caret_.row_ = caret_.row_;
}

void FQTermBuffer::endDecode() {
  if (last_saved_caret_.row_ < num_rows_) {
    FQTermTextLine *line = text_lines_[last_saved_caret_.row_ + num_hist_lines_];

    line->safelySetDirtyFlag(last_saved_caret_.column_,
                           last_saved_caret_.column_ + 1);
  }

  if (caret_.row_ < num_rows_) {
    FQTermTextLine *line = text_lines_.at(caret_.row_ + num_hist_lines_);

    line->safelySetDirtyFlag(caret_.column_, caret_.column_ + 1);
  }

  clearSelect();
}

void FQTermBuffer::setMode(TermMode mode) {
  FQ_TRACE("term", 8) << "set mode " << mode;  
  
  switch (mode) {
    case INSERT_MODE:
      is_insert_mode_ = true;
      break;
    case CURSOR_MODE:
      is_cursor_mode_ = true;
      break;
    case ANSI_MODE:
      is_ansi_mode_ = true;
    case NUMERIC_MODE:
      is_numeric_mode_ = true;
      break;
    case SMOOTH_MODE:
      is_smoothscroll_mode_ = true;
      break;
    case NEWLINE_MODE:
      is_newline_mode_ = true;
      break;
    case ORIGIN_MODE:
      is_origin_mode_ = true;
      this->moveCaretTo(0, top_row_);
      break;
    case AUTOWRAP_MODE:
      is_autowrap_mode_ = true;
      break;
    case AUTOREPEAT_MODE:
      is_autorepeat_mode_ = true;
      break;
    case LIGHTBG_MODE:
      is_lightbg_mode_ = true;
      break;
    default:
      break;
  }
}

void FQTermBuffer::resetMode(TermMode mode) {
  FQ_TRACE("term", 8) << "reset mode " << mode;
  
  switch (mode) {
    case INSERT_MODE:
      is_insert_mode_ = false;
      break;
    case CURSOR_MODE:
      is_cursor_mode_ = false;
      break;
    case ANSI_MODE:
      is_ansi_mode_ = false;
    case NUMERIC_MODE:
      is_numeric_mode_ = false;
      break;
    case SMOOTH_MODE:
      is_smoothscroll_mode_ = false;
      break;
    case NEWLINE_MODE:
      is_newline_mode_ = false;
      break;
    case ORIGIN_MODE:
      is_origin_mode_ = false;
      this->moveCaretTo(0, 0);
      break;
    case AUTOWRAP_MODE:
      is_autowrap_mode_ = false;
      break;
    case AUTOREPEAT_MODE:
      is_autorepeat_mode_ = false;
      break;
    case LIGHTBG_MODE:
      is_lightbg_mode_ = false;
      break;
    default:
      break;
  }
}

void FQTermBuffer::setSelect(const QPoint &pt1, const QPoint &pt2) {
  QPoint ptSelStart, ptSelEnd;

  if (pt1.y() == pt2.y()) {
    ptSelStart = pt1.x() < pt2.x() ? pt1 : pt2;
    ptSelEnd = pt1.x() > pt2.x() ? pt1 : pt2;
  } else {
    ptSelStart = pt1.y() < pt2.y() ? pt1 : pt2;
    ptSelEnd = pt1.y() > pt2.y() ? pt1 : pt2;
  }

  int y1 = ptSelStart.y();
  int y2 = ptSelEnd.y();

  if (!(selection_start_ == QPoint(-1, -1)
        && selection_end_ == QPoint(-1, -1))) {
    y1 = qMin(y1, selection_start_.y());
    y2 = qMax(y2, selection_end_.y());
  }

  for (int i = y1; i <= y2; i++) {
      text_lines_.value(i, NULL)->setAllDirty();
  }

  selection_start_ = ptSelStart;
  selection_end_ = ptSelEnd;
}

void FQTermBuffer::clearSelect() {
  if (selection_start_ ==  QPoint(-1, -1) && selection_end_ == QPoint(-1, -1)) {
    return;
  }

  for (int i = selection_start_.y(); i <= selection_end_.y(); i++) {
    text_lines_.value(i, NULL)->setAllDirty();
  }

  selection_start_ = selection_end_ = QPoint(-1, -1);
}

bool FQTermBuffer::isSelected(int line_index) const {
  if (selection_start_ ==  QPoint(-1, -1) && selection_end_ == QPoint(-1, -1)) {
    return false;
  } else {
    return line_index >= selection_start_.y() && line_index <= selection_end_.y();
  }
}

bool FQTermBuffer::isSelected(const QPoint &cell, bool is_rect_sel) const {
  if (selection_start_ ==  QPoint(-1, -1) && selection_end_ == QPoint(-1, -1)) {
    return false;
  }

  if (cell.y() < 0 || cell.y() >= getNumLines()) {
    return false;
  }

  int x1 = selection_start_.x();
  int y1 = selection_start_.y();

  int x2 = selection_end_.x();
  int y2 = selection_end_.y();

  if (cell.y() < y1 || cell.y() > y2)
    return false;

  const FQTermTextLine *line = this->getTextLineInBuffer(cell.y());

  int cell_begin = 0;
  int cell_end = cell.x() + 1;

  if (is_rect_sel) {
    int minx = qMin(x1, x2);
    int maxx = qMax(x1, x2);
    cell_begin = line->getCellBegin(qMin(minx, (int)line->getWidth()));
    cell_end = line->getCellEnd(qMin(maxx + 1, (int)line->getWidth()));
  } else {
    if (cell.y() == y1) {
      cell_begin = line->getCellBegin(qMin(x1, (int)line->getWidth()));
    }
    if (cell.y() == y2) {
      cell_end = line->getCellEnd(qMin(x2 + 1, (int)line->getWidth()));
    }
  }

  return cell_begin <= cell.x() && cell.x() < cell_end;  
}

static void removeTrailSpace(QString &line) {
  for (int last_non_space = line.size() - 1;
       last_non_space >= 0; --last_non_space) {
    QChar a = line.at(last_non_space);
    if (!a.isSpace()) {
      line.resize(last_non_space + 1);
      break;
    }

    if (last_non_space == 0) {
      line.resize(0);
    }
  }
}


QString FQTermBuffer::getTextSelected(bool is_rect_sel, bool is_color_copy, 
                                     const QByteArray &escape) const {
  QString cstrSelect;
  QString strTemp;

  if (selection_start_ == QPoint(-1, -1) && selection_end_ == QPoint(-1, -1)) {
    return cstrSelect;
  }

  QRect rc;

  for (int i = selection_start_.y(); i <= selection_end_.y(); i++) {
    strTemp.clear();
    rc = getSelectRect(i, is_rect_sel);

    FQTermTextLine *line = text_lines_.at(i);
    unsigned cell_begin = qMax(rc.left(), 0);
    unsigned cell_end = qMin(rc.right() + 1, (int)line->getWidth());
    FQ_ASSERT(rc.left() + rc.width() == rc.right() + 1);

    if (cell_begin < cell_end) {
      cell_begin = line->getCellBegin(cell_begin);
      cell_end = line->getCellEnd(cell_end);

      if (is_color_copy) {
        line->getAnsiText(cell_begin, cell_end, strTemp, escape);
      } else {
        line->getPlainText(cell_begin, cell_end, strTemp);
      }
    }

    removeTrailSpace(strTemp);

    cstrSelect += strTemp;

    // add newline except the last line
    if (i != selection_end_.y()) {
      cstrSelect += OS_NEW_LINE;
    }
  }

  return cstrSelect;
}

QRect FQTermBuffer::getSelectRect(int line_index, bool is_rect_sel) const {
  FQ_ASSERT(isSelected(line_index));

  if (is_rect_sel) {
    return QRect(qMin(selection_start_.x(), selection_end_.x()), line_index,
                 abs(selection_end_.x() - selection_start_.x()) + 1, 1);
  } else if (selection_start_.y() == selection_end_.y()) {
    return QRect(selection_start_.x(), line_index,
                 qMin(selection_end_.x(), num_columns_) - selection_start_.x() + 1, 1);
  } else if (line_index == selection_start_.y()) {
    return QRect(selection_start_.x(), line_index,
                 qMax(0, num_columns_ - selection_start_.x()), 1);
  } else if (line_index == selection_end_.y()) {
    return QRect(0, line_index, qMin(num_columns_, selection_end_.x() + 1), 1);
  } else {
    return QRect(0, line_index, num_columns_, 1);
  }
}

void FQTermBuffer::scrollTerm(int numRows) {
  scrollLinesInTerm(caret_.row_, numRows);
  moveCaretOffset(0, numRows);
}


}  // namespace FQTerm

#include "fqterm_buffer.moc"
