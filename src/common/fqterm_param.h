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

#ifndef FQTERM_PARAM_H
#define FQTERM_PARAM_H

#include <QString>
#include <QColor>

namespace FQTerm {


//The FQTerm Pref also contains global settings.
//TODO: move all global settings into FQTermPref.
struct FQTermPref {
  static FQTermPref* getInstance() 
  {
    //Though Singleton, I didn't define a ctor and make it private
    //Why?
    //I'm lazy.
    static FQTermPref* pThis = new FQTermPref();
    return pThis;
  }
  int displayOffset_;
  int vsetting_;
  int imeEncodingID_; //	0--GBK	1--BIG5
  int widthToWrapWord_;
  // bool bSmartWW;
  bool isWheelSupported_;
  bool openWarnOnClose_;
  bool openTabBlinking_;
  bool replyENQ_;
  // bool bLogMsg;
  QString httpBrowser_;
  int openBeep_;
  QString beepSoundFileName_;
  int beepMethodID_;
  QString beepPlayerName_;
  bool openUrlCheck_;
  // bool bAutoCopy;
  bool openAntiAlias_;
  bool openMinimizeToTray_;
  bool needClearZmodemPoolOnClose_;
  bool useStyleSheet_;
  QString styleSheetFile_;
  QString zmodemDir_;
  QString poolDir_;
  QString imageViewerName_;
  QString externalEditor_;
  QString externalEditorArg_;
  QString searchEngine_;
  //global settings
  bool isBossColor_;
  bool isAnsciiEnhance_;
  QString escapeString_;
  int termScrollBarPosition_; // 0--hide 1--LEFT 2--RIGHT
  bool runServer_;
};


class FQTermParam {
 public:
  FQTermParam();
  FQTermParam(const FQTermParam &);
  ~FQTermParam();

  FQTermParam &operator = (const FQTermParam &);

  bool operator==(const FQTermParam &);

  static const FQTermParam& getFQBBSParam();
  static QString getLanguageName(bool isEnglish, bool translate = true);
  // General
  // Name
  QString name_;
  // Address
  QString hostAddress_;
  // Port
  quint16 port_;
  // Host Type
  int hostType_; // 0--BBS 1--*NIX
  // Auto Login
  bool isAutoLogin_;
  // Pre Login
  QString preLoginCommand_;
  // User Name
  QString userName_;
  // Password
  QString password_;
  // Post Login
  QString postLoginCommand_;

  // Display
  // {FQTERM_ENCODING_GBK = 0, FQTERM_ENCODING_BIG5 = 1, FQTERM_ENCODING_UTF8 = 2, FQTERM_ENCODING_HKSCS = 3, FQTERM_ENCODING_UAO = 4};
  int serverEncodingID_;

  // 0 - keep column/row, adjust font
  // 1 - keep font, adjust column/row
  // 2 - keep column/row/font
  int isFontAutoFit_;
  // Always Highlight
  bool isAlwaysHighlight_;
  // ANSI Color
  bool isAnsiColor_;
  // Font Name
  QString englishFontName_;
  QString otherFontName_;
  // Font Size
  int englishFontSize_;
  int otherFontSize_;

  int alignMode_; //how to align english char/chinese char in vertical direction 0 -- vcenter, 1 -- bottom 2 -- top
  int charSpacing_; //additional char spacing for English char.
  int lineSpacing_; //additional line spacing
  //int charRatio_; //height of english char / width of english char
  // Background Color
  QColor backgroundColor_;
  // Foreground Color
  QColor foregroundColor_;
  // Schema File
  QString schemaFileName_;

  // Terminal
  // Terminal Type
  QString virtualTermType_;
  // Key Type
  int keyboardType_; // 0--BBS 1--XTERM 2--VT100
  // Columns & Rows
  int numColumns_, numRows_;
  // Scroll Lines
  int numScrollLines_;
  // Curor Type
  int cursorType_; // 0--Block 1--Underline 2--I Type
  // the esacpe string
  QString escapeString_;

  //Keyboard
  int backspaceType_; //0--^H 1--^?(127)


  // Connection
  // Proxy Type
  int proxyType_; // 0--None 1--Wingate 2--SOCKS4 3--SOCKS5 4--HTTP
  // Address
  QString proxyHostName_;
  // Port
  quint16 proxyPort_;
  // Authentation
  bool isAuthentation_;
  // User Name
  QString proxyUserName_;
  // Password
  QString proxyPassword_;
  // Protocol
  int protocolType_; // 0--Telnet 1--SSH1 2--SSH2 3--Local
  // User Name
  QString sshUserName_;
  // Password
  QString sshPassword_;
  // Misc
  // Max Idle Time %s
  int maxIdleSeconds_;
  QString replyKeyCombination_;
  // Send When Idle
  QString antiIdleMessage_;
  bool isAntiIdle_;
  // wether autoreply
  bool isAutoReply_;
  // Auto Reply Messages
  QString autoReplyMessage_;
  // Reconnect When Disconnected By Host
  bool isAutoReconnect_;
  // Reconnect Interval (s)
  int reconnectInterval_;
  // Retry times
  int retryTimes_; // -1 -- infinite
  // Close Window When Disconnected By Host
  bool isAutoCloseWin_;

  bool isBeep_;
  bool isBuzz_;

  // Mouse
  bool isSupportMouse_;
  int menuType_; // 0--underline 1--reverse 2--color
  QColor menuColor_;
  // Script
  bool isAutoLoadScript_;
  QString autoLoadedScriptFileName_;
  // Zmodem
  bool enableZmodem_;

  bool isColorCopy_;
  bool isRectSelect_;
  bool isAutoCopy_;
  bool isAutoWrap_;

  private:
    void copy(const FQTermParam& parm);
};

}  // namespace FQTerm

#endif  // FQTERM_PARAM_H
