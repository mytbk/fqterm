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

#include <QByteArray>

#include "fqterm.h"
#include "fqterm_telnet.h"
#include "fqterm_decode.h"
#include "fqterm_buffer.h"
#include "fqwcwidth.h"

namespace FQTerm {

#define MODE_MouseX11	0

/************************************************************************/
// state for FSM
// please read ANSI decoding
StateOption FQTermDecode::VT100StateMachine::normal_state_[] =  {
  {
    CHAR_CR, &FQTermDecode::cr, normal_state_
  }, {
    CHAR_LF, &FQTermDecode::lf, normal_state_
  }, {
    CHAR_VT, &FQTermDecode::lf, normal_state_
  }, {
    CHAR_ESC, 0, esc_state_
  }, {
    CHAR_FF, &FQTermDecode::ff, normal_state_
  }, {
    CHAR_TAB, &FQTermDecode::tab, normal_state_
  }, {
    CHAR_BS, &FQTermDecode::bs, normal_state_
  }, {
    CHAR_BELL, &FQTermDecode::bell, normal_state_
  }, {
    CHAR_SO, &FQTermDecode::g1, normal_state_
  }, {
    CHAR_SI, &FQTermDecode::g0, normal_state_
  }, {
    CHAR_CAN, 0, normal_state_ // should be ignored
  }, {
    CHAR_DEL, &FQTermDecode::del, normal_state_ 
  }, {
    CHAR_NUL, 0, normal_state_ // should be ignored
  }, {
    CHAR_ENQ, &FQTermDecode::enq, normal_state_
  }, {
    CHAR_NORMAL, &FQTermDecode::normalInput, normal_state_
  }
};

// state after a ESC_CHAR
StateOption FQTermDecode::VT100StateMachine::esc_state_[] =  {
  {
    '7', &FQTermDecode::saveCursor, normal_state_
  }, {
    '8', &FQTermDecode::restoreCursor, normal_state_
  }, {
    '[', &FQTermDecode::clearParam, bracket_state_
  }, {
    ']', &FQTermDecode::clearParam, title_state_
  }, {
    '(', 0, select_g0_charset_state_
  }, {
    ')', 0, select_g1_charset_state_
  }, {
    'D', &FQTermDecode::moveCursorDown, normal_state_
  }, {
    'E', &FQTermDecode::nextLine, normal_state_
  }, {
    'H', &FQTermDecode::addTabStop, normal_state_
  }, {
    'M', &FQTermDecode::moveCursorUp, normal_state_
  }, {
    '>', &FQTermDecode::setNumericKeypad, normal_state_
  }, {
    '=', &FQTermDecode::setAppKeypad, normal_state_
  }, {
    '<', &FQTermDecode::test, normal_state_
  }, {
    'c', &FQTermDecode::termReset, normal_state_
  }, {
    '#', 0, sharp_state_
  }, {
    CHAR_NORMAL, 0, normal_state_
  }
};

// state after ESC [
StateOption FQTermDecode::VT100StateMachine::bracket_state_[] =  {
  {
    '0', &FQTermDecode::paramDigit, bracket_state_
  }, {
    '1', &FQTermDecode::paramDigit, bracket_state_
  }, {
    '2', &FQTermDecode::paramDigit, bracket_state_
  }, {
    '3', &FQTermDecode::paramDigit, bracket_state_
  }, {
    '4', &FQTermDecode::paramDigit, bracket_state_
  }, {
    '5', &FQTermDecode::paramDigit, bracket_state_
  }, {
    '6', &FQTermDecode::paramDigit, bracket_state_
  }, {
    '7', &FQTermDecode::paramDigit, bracket_state_
  }, {
    '8', &FQTermDecode::paramDigit, bracket_state_
  }, {
    '9', &FQTermDecode::paramDigit, bracket_state_
  }, {
    ';', &FQTermDecode::nextParam, bracket_state_
  }, {
    '?', &FQTermDecode::clearParam, private_state_
  }, {
    'A', &FQTermDecode::cursorUp, normal_state_
  }, {
    'B', &FQTermDecode::cursorDown, normal_state_
  }, {
    'C', &FQTermDecode::cursorRight, normal_state_
  }, {
    'D', &FQTermDecode::cursorLeft, normal_state_
  }, {
    'H', &FQTermDecode::cursorPosition, normal_state_
  }, {
    'J', &FQTermDecode::eraseScreen, normal_state_
  }, {
    'K', &FQTermDecode::eraseLine, normal_state_
  }, {
    'L', &FQTermDecode::insertLine, normal_state_
  }, {
    'M', &FQTermDecode::deleteLine, normal_state_
  }, {
    'P', &FQTermDecode::deleteStr, normal_state_
  }, {
    'X', &FQTermDecode::eraseStr, normal_state_
  }, {
    'f', &FQTermDecode::cursorPosition, normal_state_
  }, {
    'g', &FQTermDecode::clearTabStop, normal_state_
  }, {
    'h', &FQTermDecode::setMode, normal_state_
  }, {
    'l', &FQTermDecode::resetMode, normal_state_
  }, {
    'm', &FQTermDecode::setAttr, normal_state_
  }, {
    'q', 0, normal_state_  // ignore LED-related commands.
  }, {
    'r', &FQTermDecode::setMargins, normal_state_
  }, {
    's', &FQTermDecode::saveCursor, normal_state_
  }, {
    'u', &FQTermDecode::restoreCursor, normal_state_
  }, {
    '@', &FQTermDecode::insertStr, normal_state_
  }, {
    CHAR_CR, &FQTermDecode::cr, bracket_state_
  }, {
    CHAR_LF, &FQTermDecode::lf, bracket_state_
  }, {
    CHAR_VT, &FQTermDecode::lf, bracket_state_
  }, {
    CHAR_FF, &FQTermDecode::ff, bracket_state_
  }, {
    CHAR_TAB, &FQTermDecode::tab, bracket_state_
  }, {
    CHAR_BS, &FQTermDecode::bs, bracket_state_
  }, {
    CHAR_BELL, &FQTermDecode::bell, bracket_state_
  }, {
    CHAR_NORMAL, 0, normal_state_
  }
};

// state after ESC (
StateOption FQTermDecode::VT100StateMachine::select_g0_charset_state_[] =  {
  {
    'A', &FQTermDecode::selectG0A, normal_state_
  }, {
    'B', &FQTermDecode::selectG0B, normal_state_
  }, {
    '0', &FQTermDecode::selectG00, normal_state_
  }, {
    '1', &FQTermDecode::selectG01, normal_state_
  }, {
    '2', &FQTermDecode::selectG02, normal_state_
  }, {
    CHAR_NORMAL, 0, normal_state_
  }
};
  
// state after ESC )
StateOption FQTermDecode::VT100StateMachine::select_g1_charset_state_[] =  {
  {
    'A', &FQTermDecode::selectG1A, normal_state_
  }, {
    'B', &FQTermDecode::selectG1B, normal_state_
  }, {
    '0', &FQTermDecode::selectG10, normal_state_
  }, {
    '1', &FQTermDecode::selectG11, normal_state_
  }, {
    '2', &FQTermDecode::selectG12, normal_state_
  }, {
    CHAR_NORMAL, 0, normal_state_
  }
};

// state after ESC [ ?
StateOption FQTermDecode::VT100StateMachine::private_state_[] =  {
  {
    '0', &FQTermDecode::paramDigit, private_state_
  }, {
    '1', &FQTermDecode::paramDigit, private_state_
  }, {
    '2', &FQTermDecode::paramDigit, private_state_
  }, {
    '3', &FQTermDecode::paramDigit, private_state_
  }, {
    '4', &FQTermDecode::paramDigit, private_state_
  }, {
    '5', &FQTermDecode::paramDigit, private_state_
  }, {
    '6', &FQTermDecode::paramDigit, private_state_
  }, {
    '7', &FQTermDecode::paramDigit, private_state_
  }, {
    '8', &FQTermDecode::paramDigit, private_state_
  }, {
    '9', &FQTermDecode::paramDigit, private_state_
  }, {
    ';', &FQTermDecode::nextParam, private_state_
  }, {
    'h', &FQTermDecode::setDecPrivateMode, normal_state_
  }, {
    'l', &FQTermDecode::resetDecPrivateMode, normal_state_
  }, {
    's', &FQTermDecode::saveMode, normal_state_
  }, {
    'r', &FQTermDecode::restoreMode, normal_state_
  }, {
    CHAR_NORMAL, 0, normal_state_
  }
};

// state after ESC #
StateOption FQTermDecode::VT100StateMachine::sharp_state_[] =  {
  {
    '8', &FQTermDecode::fillScreen, normal_state_
  }, {
    CHAR_NORMAL, 0, normal_state_
  }
};

// state after ESC ]
// to change the terminal title dynamically.
// Dynamic titles
// Many people find it useful to set the title of a terminal to reflect dynamic information, 
// such as the name of the host the user is logged into, the current working directory, etc.
// This may be done by using XTerm escape sequences. The following sequences are useful in this respect:
//  ESC]0;stringBEL    Set icon name and window title to string
//  ESC]1;stringBEL    Set icon name to string
//  ESC]2;stringBEL    Set window title to string
// where ESC is the escape character (\033), and BEL is the bell character (\007).
// Currently we just ignore this.
StateOption FQTermDecode::VT100StateMachine::title_state_[] =  {
  {
    '0', &FQTermDecode::paramDigit, title_state_
  }, {
    '1', &FQTermDecode::paramDigit, title_state_
  }, {
    '2', &FQTermDecode::paramDigit, title_state_
  }, {
    '4', &FQTermDecode::paramDigit, title_state_
  }, {
    '5', &FQTermDecode::paramDigit, title_state_
  }, {
    '6', &FQTermDecode::paramDigit, title_state_
  }, {
    ';', &FQTermDecode::clearText, title_text_state_
  }, {
    CHAR_NORMAL, 0, normal_state_
  }
};

StateOption FQTermDecode::VT100StateMachine::title_text_state_[] = {
  {
    CHAR_ESC, 0, esc_state_
  }, {
    CHAR_BELL, &FQTermDecode::setTitle, normal_state_
  }, {
    CHAR_NORMAL, &FQTermDecode::collectText, title_state_
  }
};

const char *FQTermDecode::getStateName(const StateOption *state) {
  if (state == VT100StateMachine::normal_state_) {
    return "VT100StateMachine::normal_state_";
  } else if (state == VT100StateMachine::esc_state_) {
    return "VT100StateMachine::esc_state_";
  } else if (state == VT100StateMachine::bracket_state_) {
    return "VT100StateMachine::bracket_state_";
  } else if (state == VT100StateMachine::private_state_) {
    return "VT100StateMachine::private_state_";
  } else if (state == VT100StateMachine::title_state_) {
    return "VT100StateMachine::title_state_";
  } else {
    return "Unknow";
  }
}


FQTermDecode::FQTermDecode(FQTermBuffer *buffer, int server_encoding) {
  leftToDecode_.clear();
  termBuffer_ = buffer;

  interrupt_decode_ = false;
  
  current_state_ =  VT100StateMachine::normal_state_;

  default_color_ = NO_COLOR;
  default_attr_ = NO_ATTR;

  current_color_ = default_color_;
  current_attr_ = default_attr_;
  
  isBell_ = false;

  termBuffer_->setCurrentAttr(current_color_, current_attr_);

  // TODO_UTF16: please create a encoding enum.
  server_encoding_ = server_encoding;

  currentMode_[MODE_MouseX11] = savedMode_[MODE_MouseX11] = false;
  FQ_VERIFY(connect(termBuffer_, SIGNAL(caretChangeRow()), this, SLOT(onCaretChangeRow())));
}

FQTermDecode::~FQTermDecode() {
  
}

// precess input string from telnet socket
//void FQTermDecode::ansiDecode( const QCString &cstr, int length )
int FQTermDecode::decode(const char *cstr, int length) {
  inputData_ = cstr;
  inputLength_ = length; //inputData.length();

  dataIndex_ = 0;
  isBell_ = false;

  interrupt_decode_ = false;

  int i;
  StateOption *lastState;

  termBuffer_->startDecode();

  // here we use FSM to ANSI decoding
  // use switch case is ok too
  // but i think use function pointer array can make this clear
  // you can see the defination at the beginning
  while (dataIndex_ < inputLength_) {
    // current state always be initialized to point to the deginning of three structures
    // ( normalState, escState, bracketState )
    i = 0;
    while (current_state_[i].byte != CHAR_NORMAL && current_state_[i].byte !=
           inputData_[dataIndex_]) {
      i++;
    }

    // action must be allowed to redirect state change
    // get current state with input character i ( hehe, current now become last one )
    lastState = current_state_ + i; // good !!

    bool trace_state = true;
    if (current_state_ == VT100StateMachine::normal_state_ 
        && lastState->nextState == VT100StateMachine::normal_state_) {
      trace_state = false;
    }

    if (trace_state) {
      FQ_TRACE("ansi", 10) << "Current state: " << getStateName(current_state_);
      FQ_TRACE("ansi", 10) << dumpHexString << std::string("") + inputData_[dataIndex_] 
                        << dumpNormalString << " " << inputData_[dataIndex_];
    }

    if (lastState->action != 0) {
      (this->*(lastState->action))();
    } else {
      if (trace_state) FQ_TRACE("ansi", 10) << "No action";
    }
    if (trace_state) FQ_TRACE("ansi", 10) << "Next state: " << getStateName(lastState->nextState);

    // reinit current state
    current_state_ = lastState->nextState;

    dataIndex_++;

    if (interrupt_decode_) {
      break;
    }
  }
  termBuffer_->endDecode();

  return dataIndex_;
}


QString FQTermDecode::bbs2unicode(const QByteArray &text) {
  return encoding2unicode(text, server_encoding_);
}

///////////////////////////////////
//helper function to decode utf8//
//////////////////////////////////
static int utf8_expected_byte_count(char first_byte)
{
  char expected = 0;
  if (!(first_byte & 0x80)) 
    return 0; //1 byte ascii
  else
    expected++;
  if (!(first_byte & 0x40))
  {
    return -1;  //not a leading byte.
  }
  if (!(first_byte & 0x20))
    return expected;
  else
    expected++;
  if (!(first_byte & 0x10))
    return expected;
  else
    expected++;
  if (!(first_byte & 0x08))
    return expected;
  return -1;
}

///////////////////////////////////
//helper function to decode utf8//
//////////////////////////////////
static int gdbnbig5_expected_byte_count(char first_byte)
{
  char expected = 0;
  if (!(first_byte & 0x80)) 
    return 0; //1 byte ascii
  else
    expected++;
  return expected;
}

int FQTermDecode::processInput(QByteArray& result)
{
  int charstate = FQTermTextLine::NORMAL;
  int expect_bytes = 0;
  if (leftToDecode_.size())
  {
    switch(server_encoding_)
    {
    case FQTERM_ENCODING_GBK:
    case FQTERM_ENCODING_BIG5:
    case FQTERM_ENCODING_HKSCS:
    case FQTERM_ENCODING_UAO:
      expect_bytes = gdbnbig5_expected_byte_count(leftToDecode_[0]);
      break;
    case FQTERM_ENCODING_UTF8:
      expect_bytes = utf8_expected_byte_count(leftToDecode_[0]);
      break;
    }
  }

  //TODO: error.
  if (expect_bytes < 0) {
    expect_bytes = 0;
  }
  

  int n = 0;
  int last_char_index = n;
  while (((dataIndex_ + n) < inputLength_) && (signed(inputData_[dataIndex_ + n]) >= 0x20
    || signed(inputData_[dataIndex_ + n]) < 0x00)) {

      if (expect_bytes != 0) {
        expect_bytes--;
      } else {
        last_char_index = n;
        switch(server_encoding_)
        {
        case FQTERM_ENCODING_GBK:
        case FQTERM_ENCODING_BIG5:
        case FQTERM_ENCODING_HKSCS:
        case FQTERM_ENCODING_UAO:
          expect_bytes = gdbnbig5_expected_byte_count(inputData_[dataIndex_ + n]);
          break;
        case FQTERM_ENCODING_UTF8:
          expect_bytes = utf8_expected_byte_count(inputData_[dataIndex_ + n]);
          break;
        }
        //TODO: error.
        if (expect_bytes < 0) {
          expect_bytes = 0;
        }

        
      }

      n++;
  }

  if (expect_bytes) {
    if(dataIndex_ + n == inputLength_) {
      interrupt_decode_ = true;
    } 
  }

  result.clear();
  result.reserve(n + 1);
  if (leftToDecode_.size()) {
    result += leftToDecode_;
    switch(server_encoding_)
    {
    case FQTERM_ENCODING_GBK:
    case FQTERM_ENCODING_BIG5:
    case FQTERM_ENCODING_HKSCS:
    case FQTERM_ENCODING_UAO:
      charstate |= FQTermTextLine::SECONDPART;
      break;
    case FQTERM_ENCODING_UTF8:
      break;
    }
    leftToDecode_.clear();
  }
  int real_n = n;
  if (expect_bytes) {
    real_n = last_char_index;
    leftToDecode_ = QByteArray(inputData_ + dataIndex_ + last_char_index, n - last_char_index);
  }

  result.push_back(QByteArray(inputData_ + dataIndex_, real_n));
  if (expect_bytes) {
    switch(server_encoding_)
    {
    case FQTERM_ENCODING_GBK:
    case FQTERM_ENCODING_BIG5:
    case FQTERM_ENCODING_HKSCS:
    case FQTERM_ENCODING_UAO:
      result.push_back('?');  //make sure the attr is recorded,
      //since last -1 operation can make cstr to be empty
      charstate |= FQTermTextLine::FIRSTPART;
      break;
    case FQTERM_ENCODING_UTF8:
      break;
    }
  }

  n--;
  dataIndex_ += n;

  return charstate;
}


// fill letters into char buffer
// TODO: this function may contain bug, need double-check.
//       1. input should be ascii-compitable encoding.
void FQTermDecode::normalInput() {

  FQ_FUNC_TRACE("ansi", 8);

  // TODO_UTF16: check ascii-incompitable encoding.
  if (signed(inputData_[dataIndex_]) < 0x20 && signed(inputData_[dataIndex_]) >= 0x00) {
	// not print char
    return ;
  }
  
  QByteArray cstr;
  int charstate = processInput(cstr);
  


  QString str = bbs2unicode(cstr);
  FQ_TRACE("normal_input", 9) << "hex: " << dumpHexString
                               << str.toLocal8Bit().constData() ;  
  FQ_TRACE("normal_input", 9) << "text: " << str;
  termBuffer_->writeText(str, charstate);

}

// non-printing characters functions
void FQTermDecode::cr() {
  FQ_FUNC_TRACE("ansi", 8);
  // FIXME: dirty
  termBuffer_->carriageReturn();
}

void FQTermDecode::lf() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->lineFeed();
}

void FQTermDecode::ff() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->eraseEntireTerm();

  termBuffer_->moveCaretTo(0, 0);
}

void FQTermDecode::tab() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->tab();
}

void FQTermDecode::bs() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->moveCaretOffset(-1, 0);
}

void FQTermDecode::bell() {
  FQ_FUNC_TRACE("ansi", 8);
  isBell_ = true;
}

void FQTermDecode::del() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->moveCaretOffset(-1, 0);
  //termBuffer_->deleteText(1);
}

void FQTermDecode::g0() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->invokeCharset(true);
}

void FQTermDecode::g1() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->invokeCharset(false);
}

void FQTermDecode::enq() {
  emit enqReceived();
}

void FQTermDecode::addTabStop() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->addTabStop();
}

void FQTermDecode::clearTabStop() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();

  if (param_[0] == 0) {
    termBuffer_->clearTabStop(false);
  } else if (param_[0] == 3) {
    termBuffer_->clearTabStop(true);
  }
}

void FQTermDecode::setMargins() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  if (isParamAvailable_) {
    termBuffer_->setMargins(param_[0] - 1, param_[1] - 1);
  } else {
    termBuffer_->setMargins(0, termBuffer_->getNumRows() - 1);
  }
}

    void FQTermDecode::termReset()
    {
        FQ_FUNC_TRACE("ansi", 8);
        termBuffer_->termReset();
    }
    
// parameters functions
void FQTermDecode::clearParam() {
  FQ_FUNC_TRACE("ansi", 9);
  paramIndex_ = 0;
  memset(param_, 0, sizeof(param_));
  isParamAvailable_ = false;
}

// for performance, this grabs all digits
void FQTermDecode::paramDigit() {
  FQ_FUNC_TRACE("ansi", 10);
  isParamAvailable_ = true;

  // make stream into number
  // ( e.g. this input character is '1' and this param is 4
  // after the following sentence this param is changed to 41
  param_[paramIndex_] = param_[paramIndex_] *10+inputData_[dataIndex_] - '0';
}

void FQTermDecode::nextParam() {
  FQ_FUNC_TRACE("ansi", 9);
  paramIndex_++;
}

void FQTermDecode::saveCursor() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->saveCaret();
}

void FQTermDecode::restoreCursor() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->restoreCaret();
}

void FQTermDecode::cursorPosition() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  int x = param_[1];
  int y = param_[0];

  if (x == 0) {
    x = 1;
  }
  if (y == 0) {
    y = 1;
  }

  termBuffer_->changeCaretPosition(x - 1, y - 1);
}

void FQTermDecode::cursorLeft() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  int n = param_[0];

  if (n < 1) {
    n = 1;
  }

  termBuffer_->moveCaretOffset(-n, 0);
}

void FQTermDecode::cursorRight() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  int n = param_[0];

  if (n < 1) {
    n = 1;
  }

  termBuffer_->moveCaretOffset(n, 0);
}

void FQTermDecode::cursorUp() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  int n = param_[0];

  if (n < 1) {
    n = 1;
  }

  termBuffer_->moveCaretOffset(0, -n);
}

void FQTermDecode::cursorDown() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  int n = param_[0];

  if (n < 1) {
    n = 1;
  }

  termBuffer_->moveCaretOffset(0, n);
}

void FQTermDecode::moveCursorUp() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->scrollTerm(-1);//moveCaretOffset(0, -1, true);
}

void FQTermDecode::moveCursorDown() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->scrollTerm(1);//moveCaretOffset(0, 1, true);
}

void FQTermDecode::nextLine() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->carriageReturn();
  termBuffer_->moveCaretOffset(0, 1, true);
}

void FQTermDecode::selectG0A() {
  FQ_FUNC_TRACE("ansi", 8);  
  termBuffer_->SelectVtCharacterSet(FQTermBuffer::UNITED_KINGDOM_SET, true);
}

void FQTermDecode::selectG0B() {
  FQ_FUNC_TRACE("ansi", 8);  
  termBuffer_->SelectVtCharacterSet(FQTermBuffer::ASCII_SET, true);
}

void FQTermDecode::selectG00() {
  FQ_FUNC_TRACE("ansi", 8);  
  termBuffer_->SelectVtCharacterSet(FQTermBuffer::SPECIAL_GRAPHICS, true);
}

void FQTermDecode::selectG01() {
  FQ_FUNC_TRACE("ansi", 8);  
  termBuffer_->SelectVtCharacterSet(
      FQTermBuffer::ALTERNATE_CHARACTER_ROM_STANDARD_CHARACTER_SET, true);
}

void FQTermDecode::selectG02() {
  FQ_FUNC_TRACE("ansi", 8);  
  termBuffer_->SelectVtCharacterSet(
      FQTermBuffer::ALTERNATE_CHARACTER_ROM_SPECIAL_GRAPHICS, true);
}

void FQTermDecode::selectG1A() {
  FQ_FUNC_TRACE("ansi", 8);  
  termBuffer_->SelectVtCharacterSet(FQTermBuffer::UNITED_KINGDOM_SET, false);
}

void FQTermDecode::selectG1B() {
  FQ_FUNC_TRACE("ansi", 8);  
  termBuffer_->SelectVtCharacterSet(FQTermBuffer::ASCII_SET, false);
}

void FQTermDecode::selectG10() {
  FQ_FUNC_TRACE("ansi", 8);  
  termBuffer_->SelectVtCharacterSet(FQTermBuffer::SPECIAL_GRAPHICS, false);
}

void FQTermDecode::selectG11() {
  FQ_FUNC_TRACE("ansi", 8);  
  termBuffer_->SelectVtCharacterSet(
      FQTermBuffer::ALTERNATE_CHARACTER_ROM_STANDARD_CHARACTER_SET, false);
}

void FQTermDecode::selectG12() {
  FQ_FUNC_TRACE("ansi", 8);  
  termBuffer_->SelectVtCharacterSet(
      FQTermBuffer::ALTERNATE_CHARACTER_ROM_SPECIAL_GRAPHICS, false);
}

// erase functions
void FQTermDecode::eraseStr() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  int n = param_[0];

  if (n < 1) {
    n = 1;
  }

  termBuffer_->eraseText(n);
}

// insert functions
void FQTermDecode::insertStr() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  int n = param_[0];

  if (n < 1) {
    n = 1;
  }

  termBuffer_->insertSpaces(n);
}

// delete functions
void FQTermDecode::deleteStr() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  int n = param_[0];

  if (n < 1) {
    n = 1;
  }

  termBuffer_->deleteText(n);
}

void FQTermDecode::eraseLine() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  switch (param_[0]) {
    case 0:
      termBuffer_->eraseToLineEnd();
      break;
    case 1:
      termBuffer_->eraseToLineBegin();
      break;
    case 2:
      termBuffer_->eraseEntireLine();
      break;
    default:
      break;
  }
}

void FQTermDecode::insertLine() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();

  int n = param_[0];

  if (n < 1) {
    n = 1;
  }

  termBuffer_->insertLines(n);
}

void FQTermDecode::deleteLine() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  int n = param_[0];

  if (n < 1) {
    n = 1;
  }

  termBuffer_->deleteLines(n);
}

void FQTermDecode::eraseScreen() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  switch (param_[0]) {
    case 0:
      termBuffer_->eraseToTermEnd();
      break;
    case 1:
      termBuffer_->eraseToTermBegin();
      break;
    case 2:
      termBuffer_->eraseEntireTerm();
      break;
    case 3:
      break;
  }
}

void FQTermDecode::setAttr() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  // get all attributes of character
  if (!paramIndex_ && param_[0] == 0) {
    current_color_ = default_color_;
    current_attr_ = default_attr_;

    termBuffer_->setCurrentAttr(current_color_, current_attr_);
    return ;
  }

  unsigned char cp = current_color_;
  unsigned char ea = current_attr_;
  for (int n = 0; n <= paramIndex_; n++) {
    if (param_[n] / 10 == 4) {
      // background color
      cp = cp &~BGMASK;
      cp += SETBG(param_[n] % 10);
    } else if (param_[n] / 10 == 3) {
      // front color
      cp = cp &~FGMASK;
      cp += SETFG(param_[n] % 10);
    } else {
      switch (param_[n]) {
        case 0:
          // attr off
          cp = default_color_; //NO_COLOR;
          ea = default_attr_; //NO_ATTR;
          break;
        case 1:
          // bright
          ea = SETBRIGHT(ea);
          break;
        case 2:
          // dim
          ea = SETDIM(ea);
          break;
        case 4:
          // underline
          ea = SETUNDERLINE(ea);
          break;
        case 5:
          // blink
          ea = SETBLINK(ea);
          break;
        case 7:
          // reverse
          ea = SETREVERSE(ea);
          break;
        case 8:
          // invisible
          ea = SETINVISIBLE(ea);
          break;
        default:
          break;
      }
    }
  }

  current_color_ = cp;
  current_attr_ = ea;

  termBuffer_->setCurrentAttr(current_color_, current_attr_);
}

void FQTermDecode::setMode() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();

  for (int i = 0; i <= paramIndex_; i++) {
    int n = param_[i];
    switch (n) {
      case 4:
        termBuffer_->setMode(FQTermBuffer::INSERT_MODE);
        break;
      case 20:
        termBuffer_->setMode(FQTermBuffer::NEWLINE_MODE);
        break;
      default:
        break;
    }
  }
}

void FQTermDecode::resetMode() {
  //TODO: more modes here.
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  for (int i = 0; i <= paramIndex_; i++) {
    int n = param_[i];
    switch (n) {
      case 4:
        termBuffer_->resetMode(FQTermBuffer::INSERT_MODE);
        break;
      case 20:
        termBuffer_->resetMode(FQTermBuffer::NEWLINE_MODE);
        break;
      default:
        break;
    }
  }
}

void FQTermDecode::setDecPrivateMode() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  for (int i = 0; i <= paramIndex_; i++) {
    int n = param_[i];
    switch (n) {
      case 1:
        termBuffer_->setMode(FQTermBuffer::CURSOR_MODE);
        break;
      case 2:
        termBuffer_->setMode(FQTermBuffer::ANSI_MODE);
        break;
      case 3:
	  termBuffer_->setTermSize(132, 24);
        break;
      case 4:
        // smooth scrolling mode
        // nothing to do.
        break;
      case 5:
        termBuffer_->setMode(FQTermBuffer::LIGHTBG_MODE);
        break;
      case 6:
        termBuffer_->setMode(FQTermBuffer::ORIGIN_MODE);
        break;
      case 7:
        termBuffer_->setMode(FQTermBuffer::AUTOWRAP_MODE);
        break;
      case 8:
        termBuffer_->setMode(FQTermBuffer::AUTOREPEAT_MODE);
        break;
      case 9:
        // Interlace mode
        // nothing to do.
        break;
      case 1000:
      case 1001:
        emit mouseMode(true);
        currentMode_[MODE_MouseX11] = true;
        break;
      default:
        break;
    }
  }
}

void FQTermDecode::resetDecPrivateMode() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  for (int i = 0; i <= paramIndex_; i++) {
    int n = param_[i];
    switch (n) {
      case 1:
        termBuffer_->resetMode(FQTermBuffer::CURSOR_MODE);
        break;
      case 2:
        termBuffer_->resetMode(FQTermBuffer::ANSI_MODE);
        break;
      case 3:
        termBuffer_->setTermSize(80, 24);
        break;
      case 4:
        // jump scrolling mode
        // nothing to do.
        break;
      case 5:
        termBuffer_->resetMode(FQTermBuffer::LIGHTBG_MODE);
        break;
      case 6:
        termBuffer_->resetMode(FQTermBuffer::ORIGIN_MODE);
        break;
      case 7:
        termBuffer_->resetMode(FQTermBuffer::AUTOWRAP_MODE);
        break;
      case 8:
        termBuffer_->resetMode(FQTermBuffer::AUTOREPEAT_MODE);
        break;
      case 9:
        // Interlace mode
        // nothing to do.
        break;
      case 1000:
      case 1001:
        currentMode_[MODE_MouseX11] = false;
        emit mouseMode(false);
        break;
      default:
        break;
    }
  }
}

void FQTermDecode::setNumericKeypad() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->setMode(FQTermBuffer::NUMERIC_MODE);
}

void FQTermDecode::setAppKeypad() {
  FQ_FUNC_TRACE("ansi", 8);
  termBuffer_->resetMode(FQTermBuffer::NUMERIC_MODE);
}

void FQTermDecode::saveMode() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();

  for (int i = 0; i <= paramIndex_; i++) {
    int n = param_[i];
    switch (n) {
      case 1000:
      case 1001:
        savedMode_[MODE_MouseX11] = currentMode_[MODE_MouseX11];
        break;
      default:
        break;
    }
  }
}

void FQTermDecode::restoreMode() {
  FQ_FUNC_TRACE("ansi", 8);
  logParam();
  
  for (int i = 0; i <= paramIndex_; i++) {
    int n = param_[i];
    switch (n) {
      case 1000:
      case 1001:
        currentMode_[MODE_MouseX11] = savedMode_[MODE_MouseX11];
        emit mouseMode(currentMode_[MODE_MouseX11]);
        break;
      default:
        break;
    }
  }
}

void FQTermDecode::fillScreen() {
  FQ_FUNC_TRACE("ansi", 8);

  termBuffer_->fillScreenWith('E');
}

void FQTermDecode::test() {
  FQ_FUNC_TRACE("ansi", 8);
}

void FQTermDecode::logParam() const {
  if (isParamAvailable_) {
    for (int i = 8; i <= paramIndex_; ++i) {
      FQ_TRACE("ansi", 8) << "Parameter[" << i << "] = " << param_[i];
    }
  } else {
    FQ_TRACE("ansi", 8) << "No parameter.";
  }
}

void FQTermDecode::onCaretChangeRow() {
  leftToDecode_.clear();
}

void FQTermDecode::setTitle() {
  if (!isParamAvailable_) {
    return;
  }
  switch(param_[0])
  {
  case 0:
    //icon and text
    break;
  case 1:
    //icon
    break;
  case 2:
    //text
    emit onTitleSet(bbs2unicode(textParam_));
    break;
  case 46:
    //log file
    break;
  case 50:
    //font
    break;
  default:
    break;
  }
}

void FQTermDecode::collectText() {
  FQ_FUNC_TRACE("ansi", 10);
  if (textParam_.length() < 4096)
    textParam_ += inputData_[dataIndex_];
}

void FQTermDecode::clearText() {
  FQ_FUNC_TRACE("ansi", 10);
  textParam_.clear();
}




}  // namespace FQTerm

#include "fqterm_decode.moc"
