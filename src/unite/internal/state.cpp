#include "fqterm.h"
#include "fqterm_path.h"
#include <algorithm>
#include "state.h"
#include <QDomDocument>

namespace FQTerm{

int FQTermUniteMenu::addEntry( char key )
{
  entries_.push_back(tolower(key));
  return entries_.size() - 1;
}

int FQTermUniteMenu::findEntry( char key )
{
  std::vector<char>::iterator it = std::find(entries_.begin(), entries_.end(), tolower(key));
  if (it == entries_.end()) {return -1;}
  return std::distance(entries_.begin(), it);
}

void FQTermUniteMenu::clearEntry()
{
  entries_.clear(); 
  clearPointer(current_); 
  current_ = 0;
}

int FQTermUniteMenu::currentEntry()
{
  return current_;
}

int FQTermUniteMenu::setCurrentEntry( char key )
{
  int next = findEntry(key); 
  if (next == -1)
    return currentEntry();
  clearPointer(current_); 
  current_ = next; 
  drawPointer(current_); 
  return current_;
}

int FQTermUniteMenu::setCurrentIndex( int index )
{
  clearPointer(current_); 
  current_ = index; 
  drawPointer(current_); 
  return current_;
}

int FQTermUniteMenu::nextEntry()
{
  if (!entries_.size())
    return current_;
  clearPointer(current_); 
  current_++; 
  current_ %= entries_.size(); 
  drawPointer(current_); 
  return current_;
}

int FQTermUniteMenu::prevEntry()
{
  if (!entries_.size())
    return current_;
  clearPointer(current_); 
  current_--; 
  current_ += entries_.size(); 
  current_ %= entries_.size(); 
  drawPointer(current_); 
  return current_;
}


StateOption FQTermUniteDecode::EscStateMachine::normal_state_[] =  {
  {
    CHAR_ESC, 0, esc_state_
  }, {
    CHAR_NORMAL, &FQTermUniteDecode::normalInput, normal_state_
  }
};

// state after a ESC_CHAR
StateOption FQTermUniteDecode::EscStateMachine::esc_state_[] =  {
  {
    '[', &FQTermUniteDecode::clearParam, bracket_state_
  }, {
    CHAR_ESC, &FQTermUniteDecode::doubleEsc, normal_state_
  },{
    CHAR_NORMAL, 0, normal_state_
    }
};

// state after ESC [
StateOption FQTermUniteDecode::EscStateMachine::bracket_state_[] =  {
  {
  '0', &FQTermUniteDecode::paramDigit, bracket_state_
  }, {
  '1', &FQTermUniteDecode::paramDigit, bracket_state_
  }, {
  '2', &FQTermUniteDecode::paramDigit, bracket_state_
  }, {
  '3', &FQTermUniteDecode::paramDigit, bracket_state_
  }, {
  '4', &FQTermUniteDecode::paramDigit, bracket_state_
  }, {
  '5', &FQTermUniteDecode::paramDigit, bracket_state_
  }, {
  '6', &FQTermUniteDecode::paramDigit, bracket_state_
  }, {
  '7', &FQTermUniteDecode::paramDigit, bracket_state_
  }, {
  '8', &FQTermUniteDecode::paramDigit, bracket_state_
  }, {
  '9', &FQTermUniteDecode::paramDigit, bracket_state_
  }, {
  'A', &FQTermUniteDecode::cursorUp, normal_state_
  }, {
  'B', &FQTermUniteDecode::cursorDown, normal_state_
  }, {
  'C', &FQTermUniteDecode::cursorRight, normal_state_
  }, {
  'D', &FQTermUniteDecode::cursorLeft, normal_state_
  }, {
  '~', &FQTermUniteDecode::functionKey, normal_state_
  }, {
    CHAR_NORMAL, 0, normal_state_
  }
};

int FQTermUniteDecode::processInput(const QByteArray& input) {
  data_ = input.data();
  currentByte_ = 0;
  inputLength_ = input.size();
  /*
  for (int i = 0; i < input.size(); ++i) {
  bool res = keyReceived(input[i]);
  if (!res) {
  return i + 1;
  }
  }
  */

  int i;
  StateOption *lastState;

  while (currentByte_ < inputLength_) {
    // current state always be initialized to point to the beginning of three structures
    // ( normalState, escState, bracketState )
    i = 0;
    while (current_state_[i].byte != CHAR_NORMAL && current_state_[i].byte !=
      data_[currentByte_]) {
        i++;
    }

    lastState = current_state_ + i; 

    bool stopDecode=false;
    if (lastState->action != 0) {
      stopDecode = (this->*(lastState->action))();
    } 

    // reinit current state
    current_state_ = lastState->nextState;

    currentByte_++;
    if (stopDecode)
      break;

  }

  return currentByte_;
}

FQTermUniteDecode::FQTermUniteDecode() {
  current_state_ = EscStateMachine::normal_state_;
  data_ = NULL;
  inputLength_ = 0;
  currentByte_ = 0;
}

bool FQTermUniteDecode::clearParam() {
  param_ = -1;
  return true;
}

bool FQTermUniteDecode::cursorLeft() {
  return keyReceived(LEFT);
}

bool FQTermUniteDecode::cursorDown() {
  return keyReceived(DOWN);
}

bool FQTermUniteDecode::cursorRight() {
  return keyReceived(RIGHT);
}

bool FQTermUniteDecode::cursorUp() {
  return keyReceived(UP);
}

bool FQTermUniteDecode::normalInput() {
  return keyReceived(data_[currentByte_]);
}

bool FQTermUniteDecode::doubleEsc() {
  return keyReceived(DOUBLEESC);
}

bool FQTermUniteDecode::paramDigit() {
  if (param_ < 0) {
    param_ = 0;
  }
  param_ = param_ * 10 + (data_[currentByte_] - '0');
  return true;
}

bool FQTermUniteDecode::functionKey() {
  int key;
  switch (param_) {
    case 1: key = HOME; break;
    case 3: key = DELETE; break;
    case 4: key = END; break;
    case 5: key = PGUP; break;
    case 6: key = PGDOWN; break;
    case 11: key = F1; break;
    case 12: key = F2; break;
    case 13: key = F3; break;
    case 14: key = F4; break;
    case 15: key = F5; break;
    case 17: key = F6; break;
    case 18: key = F7; break;
    case 19: key = F8; break;
    case 20: key = F9; break;
    case 21: key = F10; break;
    case 23: key = F11; break;
    case 24: key = F12; break;
    default:
      break;
  }
  return keyReceived(key);
}

FQTermUniteState::FQTermUniteState(FQTermUniteSessionContext* context) : context_(context) {

}

void FQTermUniteState::write( const QByteArray& output ) {
  context_->write(output);
}

QByteArray FQTermUniteState::escape() {
  return "\x1b";
}

void FQTermUniteState::moveCursorRelative(int dx, int dy) {
  //Esc[ValueA  Move cursor up n lines  CUU  
  //Esc[ValueB  Move cursor down n lines  CUD  
  //Esc[ValueC  Move cursor right n lines  CUF  
  //Esc[ValueD  Move cursor left n lines  CUB  
  if (dy > 0) {
    write(escape() + QString("[%1").arg(dy).toLocal8Bit() + "B");
  }
  if (dy < 0) {
    write(escape() + QString("[%1").arg(-dy).toLocal8Bit() + "A");
  }
  if (dx > 0) {
    write(escape() + QString("[%1").arg(dx).toLocal8Bit() + "C");
  }
  if (dx < 0) {
    write(escape() + QString("[%1").arg(-dx).toLocal8Bit() + "D");
  }

}


void FQTermUniteState::moveCursorAbsolute(int x, int y) {
  //Esc[Line;ColumnH  Move cursor to screen location v,h  CUP 
  write(escape() + QString("[%1;%2H").arg(y).arg(x).toLocal8Bit());
}

void FQTermUniteState::moveCursorHome() {
  write(escape() + "[H");
}

void FQTermUniteState::scrollWindow(int dy) {
  //EscD  Move/scroll window up one line  IND  
  //EscM  Move/scroll window down one line  RI  

  if (dy > 0) {
    for (int i = 0; i < dy; ++i)
      write(escape() + "D");
  }
  if (dy < 0) {
    dy = -dy;
    for (int i = 0; i < dy; ++i)
      write(escape() + "M");
  }
}

void FQTermUniteState::moveCursorNextLine() {
  //EscE  Move to next line  NEL  
  write(escape() + "E");

}

void FQTermUniteState::saveCursor() {
  //Esc7  Save cursor position and attributes  DECSC  
  write(escape() + "7"); 
}

void FQTermUniteState::restoreCursor() {
  //Esc8  Restore cursor position and attributes  DECSC 
  write(escape() + "8");
}

void FQTermUniteState::setAttr(int attr) {
  if (attr == -1) {
    write(escape() + "[m");
  } else {
    write(escape() + QString("[%1m").arg(attr).toLocal8Bit());
  }
}

void FQTermUniteState::setForegroundColor(int color, bool highlight /*= true*/) {
  int hl = highlight?1:0;
  write(escape() + QString("[%1;3%2m").arg(hl).arg(color).toLocal8Bit());
}

void FQTermUniteState::setBackgroundColor(int color) {
  write(escape() + QString("[4%1m").arg(color).toLocal8Bit());
}

void FQTermUniteState::clearLine(int part) {
  write(escape() + QString("[%1K").arg(part).toLocal8Bit());
}

void FQTermUniteState::clearScreen(int part) {
  write(escape() + QString("[%1J").arg(part).toLocal8Bit());
}

void FQTermUniteState::setWindowRange(int startLine, int endLine) {
  write(escape() + QString("[%1;%2r").arg(startLine).arg(endLine).toLocal8Bit());
}


QByteArray FQTermUniteState::cursorStr() {
  QChar c(0x25c6);
  QString str(c);
  return str.toUtf8();
}
FQTermWelcomeState::FQTermWelcomeState( FQTermUniteSessionContext* context ) : FQTermUniteState(context) {
  addEntry('h');
  addEntry('x');
}

void FQTermWelcomeState::initialize() {
  moveCursorHome();
  clearScreen(WHOLESCREEN);
  
  

  QFile welcomeFile(getPath(RESOURCE) + "unite/welcome");
  welcomeFile.open(QIODevice::ReadOnly);
  QByteArray str = welcomeFile.readAll();
  write(str);
  setCurrentIndex(currentEntry());
  return;
}

bool FQTermWelcomeState::keyReceived(int key) {
  if (tolower(key) == 'j' || key == UP) {
    prevEntry();
  } else if (tolower(key) == 'k' || key == DOWN) {
    nextEntry();
  } else if (key == '\r' || key == RIGHT) {
    enterEntry();
    return false;
  } else if (tolower(key) <= 'z' && tolower(key) >= 'a') {
    setCurrentEntry(key);
  } 
  return true;
}

void FQTermWelcomeState::enterEntry() {
  if (currentEntry() == 1) {
    setNextState(FQTermUniteSessionContext::EXITING);
  } else if (currentEntry() == 0) {
    setNextState(FQTermUniteSessionContext::HELP);
  }
}

void FQTermWelcomeState::clearPointer(int index) {
  moveCursorAbsolute(7, 11 + index);
  write("  ");
  moveCursorHome();
}

void FQTermWelcomeState::drawPointer(int index) {
  moveCursorAbsolute(7, 11 + index);
  write(cursorStr());
  moveCursorHome();
}



void FQTermExitState::initialize() {
  clearScreen(WHOLESCREEN);
  QFile welcomeFile(getPath(RESOURCE) + "unite/exit");
  welcomeFile.open(QIODevice::ReadOnly);
  QByteArray str = welcomeFile.readAll();
  write(str);
  setCurrentIndex(0);
}

void FQTermExitState::clearPointer(int index) {
  moveCursorAbsolute(28, 10 + index);
  setBackgroundColor(BLUE);
  write("  ");
  //moveCursorHome();
}

void FQTermExitState::drawPointer( int index ) {
  moveCursorAbsolute(28, 10 + index);
  setBackgroundColor(BLUE);
  write(cursorStr());
  //moveCursorHome();
}

void FQTermExitState::enterEntry() {
  if (currentEntry() == 1) {
    quit();
  } else {
    setNextState(FQTermUniteSessionContext::WELCOME);
  }
}

bool FQTermExitState::keyReceived( int key )
{
  if (tolower(key) == 'j' || key == UP) {
    prevEntry();
  } else if (tolower(key) == 'k' || key == DOWN) {
    nextEntry();
  } else if (key == '\r') {
    enterEntry();
    return false;
  } else if (key >= '0' && key <= '9') {
    setCurrentEntry(key);
  } else if (tolower(key) == 'c') {
    setCurrentIndex(0);
    enterEntry();
    return false;
  }
  //moveCursorHome();
  return true;
}

FQTermExitState::FQTermExitState( FQTermUniteSessionContext* context ) : FQTermUniteState(context) {
  addEntry('1');
  addEntry('2');
}

SplittedArticleBuffer::SplittedArticleBuffer(const QByteArray& utf8Content, int column, int row) {
  std::vector<QString> strList;
  QString content = QString::fromUtf8(utf8Content);
  lines_.reserve(row);
  int start = 0;
  for(;;) {
    int newStart = findNextLine(content, start, column);
    if (newStart == -1) {
      lines_.push_back(content.mid(start).toUtf8());
      break;
    } else {
      lines_.push_back(content.mid(start, newStart - start).toUtf8());
      start = newStart;
    }
  }
}

int SplittedArticleBuffer::findNextLine(const QString& str, int start, int column) {
  int count = 0;
  for (int i = start; i < str.length(); ++i) {
    if (str[i] == '\t') {
      count += 2;
    } else {
      count += str[i] <= 0x7F ? 1 : 2;
    }
    if (count > column) {
      return i;
    }
    if (str[i] == '\n') {
      if (i + 1 < str.length())
        return i + 1;
    }
  }
  return -1;
}

void FQTermReadingState::initialize() {
  clearScreen(WHOLESCREEN);
  moveCursorHome();
  drawLines(row() - 1, 1);
  drawFooter();
  moveCursorAbsolute(column(), row());
}


void FQTermReadingState::setFile(const QString& filename) {
  currentStartLine_ = 0;
  QFile file(filename);
  file.open(QIODevice::ReadOnly);
  content_ = file.readAll();
  delete saBuffer_;
  saBuffer_ = new SplittedArticleBuffer(content_, column(), row());
}

FQTermReadingState::FQTermReadingState( FQTermUniteSessionContext* context ) : FQTermUniteState(context) {
  returnState_ = FQTermUniteSessionContext::WELCOME;
  currentStartLine_ = 0;
  saBuffer_ = 0;
}

bool FQTermReadingState::keyReceived( int key ) {
  if (tolower(key) == 'q' || key == LEFT || !saBuffer_) {
    setNextState(returnState_);
    return false;
  } else if (tolower(key) == 'k' || key == DOWN) {
    adjustLineByDiff(1);
    drawLines(row() - 1, 1);
    drawFooter();
  } else if (tolower(key) == 'j' || key == UP) {
    adjustLineByDiff(-1);
    drawLines(row() - 1, 1);
    drawFooter();
  } else if (key == PGDOWN || key == ' ' || key == RIGHT) {
    adjustLineByDiff(row() - 2);
    drawLines(row() - 1, 1);
    drawFooter();
  } else if (key == PGUP) {
    adjustLineByDiff(-row() + 2);
    drawLines(row() - 1, 1);
    drawFooter();
  } else if (key == HOME) {
    adjustLineByDiff(-saBuffer_->count());
    drawLines(row() - 1, 1);
    drawFooter();
  } else if (key == END) {
    adjustLineByDiff(saBuffer_->count());
    drawLines(row() - 1, 1);
    drawFooter();
  }
  return true;
}

void FQTermReadingState::drawFooter() {
  if (!saBuffer_)
    return;
  cursorGuard(*this);
  moveCursorAbsolute(1, row());
  setBackgroundColor(BLUE);
  QByteArray footer = QString("Current Line: (%1-%2)  Total: %3").arg(currentStartLine_ + 1).arg(qMin(saBuffer_->count(), currentStartLine_ + row() - 1)).arg(saBuffer_->count()).toLocal8Bit();
  footer += QByteArray(column() - footer.size(), ' ');
  write(footer);
  setAttr(CLEARATTR);
}

void FQTermReadingState::adjustLineByDiff(int delta) {
  if (!saBuffer_)
    return;
  currentStartLine_ += delta;
  if (currentStartLine_ + row() - 1 >= saBuffer_->count())
    currentStartLine_ = saBuffer_->count() - (row() - 1);
  if (currentStartLine_ < 0)
    currentStartLine_ = 0;
}


void FQTermReadingState::drawLines(int count, int yPos) {
  if (!saBuffer_)
    return;
  cursorGuard cg(*this);
  QByteArray blankLine(column(), ' ');
  for (int i = 0; i < count; ++i) {
    moveCursorAbsolute(1, yPos + i);
    clearLine(WHOLELINE);
    if (currentStartLine_ + yPos + i - 1 >= saBuffer_->count()) {
      write(blankLine);
    } else {
      write(saBuffer_->retriveLine(currentStartLine_ + yPos + i - 1));
    }
  }
}

FQTermReadingState::~FQTermReadingState() {
  delete saBuffer_;
}





FQTermHelpState::FQTermHelpState(FQTermUniteSessionContext* context) : FQTermUniteState(context) {
  addEntry('s');  //0.entry for script
  addEntry('t');  //1.entry for shortcut
  addEntry('u');  //2.entry for unix host
  addEntry('x');  //3.entry for quit to welcome
}

void FQTermHelpState::initialize() {
  moveCursorHome();
  clearScreen(WHOLESCREEN);
  QFile welcomeFile(getPath(RESOURCE) + "unite/help");
  welcomeFile.open(QIODevice::ReadOnly);
  QByteArray str = welcomeFile.readAll();
  write(str);
  setCurrentIndex(currentEntry());
  return;
}

bool FQTermHelpState::keyReceived(int key) {
  if (tolower(key) == 'j' || key == UP) {
    prevEntry();
  } else if (tolower(key) == 'k' || key == DOWN) {
    nextEntry();
  } else if (key == '\r' || key == RIGHT) {
    enterEntry();
    return false;
  } else if (tolower(key) <= 'z' && tolower(key) >= 'a') {
    setCurrentEntry(key);
  } 
  return true;
}

void FQTermHelpState::enterEntry() {
  if (currentEntry() == 3) {
    setNextState(FQTermUniteSessionContext::WELCOME);
    return;
  }

  QString doc;
  switch(currentEntry())
  {
  case 0:
    doc = "script-doc";
    break;
  case 1:
    doc = "shortcut-doc";
    break;
  case 2:
    doc = "ssh-unix-doc";
    break;
  }
  FQTermUniteState* reader = setNextState(FQTermUniteSessionContext::READING, false);
  ((FQTermReadingState*)reader)->setFile(getPath(RESOURCE) + "unite/" + doc);
  ((FQTermReadingState*)reader)->setReturnState(FQTermUniteSessionContext::HELP);
  reader->initialize();
}

void FQTermHelpState::clearPointer(int index) {
  moveCursorAbsolute(7, 11 + index);
  write("  ");
  moveCursorHome();
}

void FQTermHelpState::drawPointer(int index) {
  moveCursorAbsolute(7, 11 + index);
  write(cursorStr());
  moveCursorHome();
}

} // namespace FQTerm

#include "state.moc"
