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
#include "fqterm_shortcuthelper.h"
#include "fqterm_config.h"
#include "fqterm_path.h"

#include <QWidget>
#include <QAction>
#include <QPixmap>
namespace FQTerm
{

FQTermShortcutHelper::FQTermShortcutHelper(FQTermConfig* config, QWidget* actionParent) :
  config_(config),
  actionParent_(actionParent)
{
  initShortcutDescriptionTable();
}

QString FQTermShortcutHelper::getShortcutText(int shortcut)
{
  if (!config_)
    return "";
  QString result = getShortcutConfig(shortcut);
  if (result == "")
    result = getShortcutDefaultText(shortcut);
  else if (result == "Undefined")
    result = "";
  return result;
}

void FQTermShortcutHelper::initShortcutDescriptionTable()
{
  initShortcutDescriptionTableEntry(CONNECT, "connect", tr(""), tr("Connect Host"), "connect");
  initShortcutDescriptionTableEntry(CASCADEWINDOWS, "cascadewindows", tr(""), tr("Cascade Windows"));
  initShortcutDescriptionTableEntry(TILEWINDOWS, "tilewindows", tr(""), tr("Tils Windows"));
  initShortcutDescriptionTableEntry(DISCONNECT, "disconnect", tr(""), tr("Disconnect Host"), "disconnect");
  initShortcutDescriptionTableEntry(ADDRESSBOOK, "addressbook", tr("F2"), tr("Address Book"), "address_book");
  initShortcutDescriptionTableEntry(QUICKLOGIN, "quicklogin", tr("F3"), tr("Quick Login"), "quick_login");
#if defined(__APPLE__)
  initShortcutDescriptionTableEntry(COPY, "copy", tr("Ctrl+C"), tr("Copy"), "copy");
  initShortcutDescriptionTableEntry(PASTE, "paste", tr("Ctrl+V"), tr("Paste"), "paste");
#else
  initShortcutDescriptionTableEntry(COPY, "copy", tr("Ctrl+Ins"), tr("Copy"), "copy");
  initShortcutDescriptionTableEntry(PASTE, "paste", tr("Shift+Insert"), tr("Paste"), "paste");
#endif 
  initShortcutDescriptionTableEntry(COPYWITHCOLOR, "copywithcolor", tr(""), tr("Copy With Color"), "copy_with_color");  
  getAction(COPYWITHCOLOR)->setCheckable(true);
  initShortcutDescriptionTableEntry(RECTANGLESELECTION, "rectangleselection", tr(""), tr("Rectangle Selection"), "rectangle_selection");
  getAction(RECTANGLESELECTION)->setCheckable(true);
  initShortcutDescriptionTableEntry(AUTOCOPYSELECTION, "autocopyselection", tr(""), tr("Auto Copy Selection"));
  getAction(AUTOCOPYSELECTION)->setCheckable(true);
  initShortcutDescriptionTableEntry(PASTEWORDWRAP, "pastewordwrap", tr(""), tr("Paste With Word Wrap"));  
  getAction(PASTEWORDWRAP)->setCheckable(true);
  initShortcutDescriptionTableEntry(ENGLISHFONT, "englishfont", tr(""), tr("Set English Font"));
  initShortcutDescriptionTableEntry(OTHERFONT, "nonenglishfont", tr(""), tr("Set Other Font"));
  initShortcutDescriptionTableEntry(COLORSETTING, "colorsetting", tr(""), tr("Color Setting"), "ansi_color");
  initShortcutDescriptionTableEntry(REFRESHSCREEN, "refreshscreen", tr(""), tr("Refresh Screen"), "refresh");
  initShortcutDescriptionTableEntry(ANSICOLOR, "ansicolor", tr(""), tr("Toggle Ansi Color"), "toggle_ansi_color");
  getAction(ANSICOLOR)->setCheckable(true);
  initShortcutDescriptionTableEntry(UIFONT, "uifont", tr(""), tr("Set UI Font"));  
  initShortcutDescriptionTableEntry(FULLSCREEN, "fullscreen", tr("F6"), tr("Toggle Full Screen"));  
  getAction(FULLSCREEN)->setCheckable(true);
  initShortcutDescriptionTableEntry(BOSSCOLOR, "bosscolor", tr("F12"), tr("Toggle Boss Color")); 
  getAction(BOSSCOLOR)->setCheckable(true);
  initShortcutDescriptionTableEntry(SWITCHBAR, "switchbar", tr(""), tr("Toggle Switch Bar"));  
  getAction(SWITCHBAR)->setCheckable(true);
  initShortcutDescriptionTableEntry(CURRENTSETTING, "currentsetting", tr(""), tr("Current Session Setting"), "preferences");  
  initShortcutDescriptionTableEntry(SEARCHIT, "googleit", tr("Ctrl+Alt+G"), tr("Google selected words"));
  initShortcutDescriptionTableEntry(WEIBOSHARE, "shareit", tr("Alt+`"), tr("Share selected text and highlighted URL to weibo"));
  initShortcutDescriptionTableEntry(EXTERNALEDITOR, "externaleditor", tr("Ctrl+Alt+E"), tr("Invoke external editor")); 
  initShortcutDescriptionTableEntry(FASTPOST, "fastpost", tr("Ctrl+Alt+F"), tr("Fast post from clipboard"));
  initShortcutDescriptionTableEntry(DEFAULTSETTING, "defaultsetting", tr(""), tr("Default Setting"));  
  initShortcutDescriptionTableEntry(PREFERENCE, "preference", tr(""), tr("Preference"), "preferences"); 
  initShortcutDescriptionTableEntry(EDITSCHEMA, "schema", tr(""), tr("Edit Schema"));
  initShortcutDescriptionTableEntry(SHORTCUTSETTING, "shortcut", tr(""), tr("Shorcut Setting"));
  initShortcutDescriptionTableEntry(COPYARTICLE, "copyarticle", tr("F9"), tr("Copy Article"), "get_article_fulltext");  
  initShortcutDescriptionTableEntry(LOGRAW, "lograw", tr(""), tr("Toggle Raw Data Logging"), "log_raw");
  getAction(LOGRAW)->setCheckable(true);
  
  initShortcutDescriptionTableEntry(ANTIIDLE, "antiidle", tr(""), tr("Toggle Anti Idle"), "anti_idle");
  getAction(ANTIIDLE)->setCheckable(true);
  
  initShortcutDescriptionTableEntry(AUTOREPLY, "autoreply", tr(""), tr("Toggle Auto Reply"), "auto_reply");  
  getAction(AUTOREPLY)->setCheckable(true);
  initShortcutDescriptionTableEntry(VIEWMESSAGE, "viewmessage", tr("F10"), tr("View Messages"), "view_messages");  
  initShortcutDescriptionTableEntry(IPLOOKUP, "iplookup", tr(""), tr("IP Lookup"));
  initShortcutDescriptionTableEntry(BEEP, "beep", tr(""), tr("Toggle Beep"), "beep");  
  getAction(BEEP)->setCheckable(true);
  initShortcutDescriptionTableEntry(MOUSESUPPORT, "mousesupport", tr(""), tr("Toggle Mouse Support"), "mouse");  
  getAction(MOUSESUPPORT)->setCheckable(true);
  initShortcutDescriptionTableEntry(IMAGEVIEWER, "imageviewer", tr(""), tr("Image Viewer"), "image_viewer");  
  initShortcutDescriptionTableEntry(RUNSCRIPT, "runscript", tr("F7"), tr("Run Script"));  
  initShortcutDescriptionTableEntry(STOPSCRIPT, "stop", tr("F8"), tr("Stop Script"));  
#ifdef HAVE_PYTHON
  initShortcutDescriptionTableEntry(RUNPYTHONSCRIPT, "runpythonscript", tr("Ctrl+F1"), tr("Run Python Script")); 
#endif //HAVE_PYTHON
  initShortcutDescriptionTableEntry(ABOUT, "about", tr("Shift+F1"), tr("About"));  
  initShortcutDescriptionTableEntry(HOMEPAGE, "homepage", tr(""), tr("Homepage"));  
  initShortcutDescriptionTableEntry(EXIT, "exit", tr(""), tr("Exit FQTerm")); 
  initShortcutDescriptionTableEntry(COLORCTL_NO, "colorctlno", tr(""), tr("Set Color Ctrl to None")); 
  getAction(COLORCTL_NO)->setCheckable(true);
  initShortcutDescriptionTableEntry(COLORCTL_SMTH, "colorctlsmth", tr(""), tr("Set Color Ctrl to **[")); 
  getAction(COLORCTL_SMTH)->setCheckable(true);
  initShortcutDescriptionTableEntry(COLORCTL_PTT, "colorctlptt", tr(""), tr("Set Color Ctrl to ^u[")); 
  getAction(COLORCTL_PTT)->setCheckable(true);
  initShortcutDescriptionTableEntry(COLORCTL_OLD_CUSTOM, "colorctloldcustom", tr(""), tr("Set Color Ctrl to old custom")); 
  getAction(COLORCTL_OLD_CUSTOM)->setCheckable(true);
  initShortcutDescriptionTableEntry(COLORCTL_CUSTOM, "colorctlcustom", tr(""), tr("Set Color Ctrl to custom")); 
  getAction(COLORCTL_CUSTOM)->setCheckable(false);
  initShortcutDescriptionTableEntry(AUTORECONNECT, "autoreconnect", tr(""), tr("Toggle Auto Reconnect"), "auto_reconnect"); 
  getAction(AUTORECONNECT)->setCheckable(true);
  initShortcutDescriptionTableEntry(SCROLLBAR_LEFT, "scrollbarleft", tr(""), tr("Set Scrollbar to Left")); 
  getAction(SCROLLBAR_LEFT)->setCheckable(true);
  initShortcutDescriptionTableEntry(SCROLLBAR_RIGHT, "scrollbarright", tr(""), tr("Set Scrollbar to Right")); 
  getAction(SCROLLBAR_RIGHT)->setCheckable(true);
  initShortcutDescriptionTableEntry(SCROLLBAR_HIDDEN, "scrollbarhidden", tr(""), tr("Set Scrollbar Hidden")); 
  getAction(SCROLLBAR_HIDDEN)->setCheckable(true);
  initShortcutDescriptionTableEntry(SEARCH_GOOGLE, "searchgoogle", tr(""), tr("Use Google")); 
  getAction(SEARCH_GOOGLE)->setCheckable(true);
  initShortcutDescriptionTableEntry(SEARCH_BAIDU, "searchbaidu", tr(""), tr("Use Baidu")); 
  getAction(SEARCH_BAIDU)->setCheckable(true);
  initShortcutDescriptionTableEntry(SEARCH_BING, "searchbing", tr(""), tr("Use Bing")); 
  getAction(SEARCH_BING)->setCheckable(true);
  initShortcutDescriptionTableEntry(SEARCH_YAHOO, "searchyahoo", tr(""), tr("Use Yahoo!")); 
  getAction(SEARCH_YAHOO)->setCheckable(true);
  initShortcutDescriptionTableEntry(SEARCH_CUSTOM, "searchcustom", tr(""), tr("Use Customized SE")); 
  getAction(SEARCH_CUSTOM)->setCheckable(true);
  initShortcutDescriptionTableEntry(LANGUAGE_ENGLISH, "languageenglish", tr(""), tr("Choose UI Language: English")); 
  getAction(LANGUAGE_ENGLISH)->setCheckable(true);
  initShortcutDescriptionTableEntry(SAVESETTING, "savesetting", tr(""), tr("Save Current Session Setting"), "save_setting");

#if defined(__APPLE__)
  QString opt(tr("Ctrl"));
#else
  QString opt(tr("Alt"));  
#endif

  initShortcutDescriptionTableEntry(NEXTWINDOW, "nextwindow", opt + tr("+Right"), tr("Next Window"));
  initShortcutDescriptionTableEntry(PREVWINDOW, "prevwindow", opt + tr("+Left"), tr("Prev Window"));

  initShortcutDescriptionTableEntry(GLOBAL_SHOW_FQTERM, "showfqterm", tr("Ctrl+Alt+Q"), tr("Show FQTerm"));
  //index, key, default shortcut, descritption

  retranslateActions();
}

void FQTermShortcutHelper::retranslateAction(int shortcut, const QString& text)
{
  if (getAction(shortcut))
    getAction(shortcut)->setText(text);
}

void FQTermShortcutHelper::retranslateActions() {
  retranslateAction(CONNECT, tr("&Connect"));
  retranslateAction(DISCONNECT, tr("&Disconnect"));
  retranslateAction(ADDRESSBOOK, tr("&Address book"));
  retranslateAction(QUICKLOGIN, tr("&Quick login"));
  retranslateAction(COPY, tr("&Copy"));
  retranslateAction(PASTE, tr("&Paste"));
  retranslateAction(COPYWITHCOLOR, tr("C&opy with color"));
  retranslateAction(RECTANGLESELECTION, tr("&Rectangle select"));
  retranslateAction(AUTOCOPYSELECTION, tr("Auto copy &select"));
  retranslateAction(PASTEWORDWRAP, tr("P&aste with wordwrap"));
  retranslateAction(ENGLISHFONT, tr("&English Font"));
  retranslateAction(OTHERFONT, tr("O&ther Font"));
  retranslateAction(COLORSETTING, tr("&Color Setting"));
  retranslateAction(ANSICOLOR, tr("&Use ANSI Color"));
  retranslateAction(REFRESHSCREEN, tr("&Refresh"));
  retranslateAction(UIFONT, tr("U&I font"));
  retranslateAction(FULLSCREEN, tr("Fullscree&n"));
  retranslateAction(BOSSCOLOR, tr("B&oss Color"));
  retranslateAction(SWITCHBAR, tr("S&witch Bar"));
  retranslateAction(SEARCHIT, tr("&Search It"));
  retranslateAction(WEIBOSHARE, tr("Sha&re It"));
  retranslateAction(EXTERNALEDITOR, tr("E&xternal Editor"));
  retranslateAction(FASTPOST, tr("&Fast Post"));
  retranslateAction(CURRENTSETTING, tr("&Setting for current session"));
  retranslateAction(DEFAULTSETTING, tr("&Default setting"));
  retranslateAction(PREFERENCE, tr("&Preferences..."));
  retranslateAction(SHORTCUTSETTING, tr("Short&cut Setting"));
  retranslateAction(EDITSCHEMA, tr("&Edit Schema"));
  retranslateAction(COPYARTICLE, tr("&Copy article"));
  retranslateAction(LOGRAW, tr("Log Raw"));
  retranslateAction(ANTIIDLE, tr("Anti &idle"));
  retranslateAction(AUTOREPLY, tr("Auto &reply"));
  retranslateAction(VIEWMESSAGE, tr("&View messages"));
  retranslateAction(IPLOOKUP, tr("I&P Lookup"));
  retranslateAction(BEEP, tr("&Beep"));
  retranslateAction(MOUSESUPPORT, tr("&Mouse support"));
  retranslateAction(IMAGEVIEWER, tr("Ima&ge viewer"));
  retranslateAction(RUNSCRIPT, tr("&Run..."));
  retranslateAction(STOPSCRIPT, tr("&Stop"));
  retranslateAction(RUNPYTHONSCRIPT, tr("Run &Python..."));
  retranslateAction(ABOUT, tr("About &FQTerm"));
  retranslateAction(HOMEPAGE, tr("FQTerm's &Homepage"));
  retranslateAction(CASCADEWINDOWS, tr("&Cascade"));
  retranslateAction(TILEWINDOWS, tr("&Tile"));
  retranslateAction(EXIT, tr("&Exit"));
  retranslateAction(COLORCTL_NO, tr("&None"));
  retranslateAction(COLORCTL_SMTH, tr("&ESC ESC ["));
  retranslateAction(COLORCTL_PTT, tr("Ctrl+&U["));
  retranslateAction(COLORCTL_CUSTOM, tr("&Custom..."));
  retranslateAction(AUTORECONNECT, tr("Reconnect When Disconnected By Host"));
  retranslateAction(NEXTWINDOW, tr("Next Window"));
  retranslateAction(PREVWINDOW, tr("Prev Window"));
  retranslateAction(LANGUAGE_ENGLISH, tr("&English"));
  retranslateAction(SCROLLBAR_HIDDEN, tr("&Hide"));
  retranslateAction(SCROLLBAR_RIGHT, tr("&Right"));
  retranslateAction(SCROLLBAR_LEFT, tr("&Left"));
  retranslateAction(SEARCH_GOOGLE, tr("&Google"));
  retranslateAction(SEARCH_BAIDU, tr("&Baidu"));
  retranslateAction(SEARCH_BING, tr("&Bing"));
  retranslateAction(SEARCH_YAHOO, tr("&Yahoo!"));
  retranslateAction(SEARCH_CUSTOM, tr("&Custom"));
  retranslateAction(SAVESETTING, tr("Save Settin&g"));
}


void FQTermShortcutHelper::resetAllShortcutText()
{
  for (int i = FQTERM_APPLICATION_SHORTCUT_START + 1; i < FQTERM_APPLICATION_SHORTCUT_END; ++i)
  {
    resetShortcutText(i);
  }
}

void FQTermShortcutHelper::resetShortcutText(int shortcut)
{
  if (!config_)
    return;
  setShortcutText(shortcut, getShortcutDefaultText(shortcut));
}

void FQTermShortcutHelper::setShortcutText(int shortcut, const QString& text)
{
  if (!config_)
    return;
  QString val = text.trimmed();
  if (val == "")
    val = "Undefined";
  setShortcutConfig(shortcut, val);
  if (getAction(shortcut))
    getAction(shortcut)->setShortcut(val);
}

FQTermShortcutHelper::~FQTermShortcutHelper()
{
}

QString FQTermShortcutHelper::getShortcutConfig(int shortcut)
{
  return config_->getItemValue(getShortcutSection(), getShortcutKey(shortcut));
}

void FQTermShortcutHelper::setShortcutConfig(int shortcut, const QString& text)
{
  config_->setItemValue(getShortcutSection(), getShortcutKey(shortcut), text);
}

void FQTermShortcutHelper::initShortcutDescriptionTableEntry(int index, const QString& key, const QString& defaultshortcuttext, const QString& description, const QString& actionSkin) {
  shortcutDescriptionTable_[index] = ShortcutDescriptionEntry(key, defaultshortcuttext, description);
  shortcutDescriptionTable_[index].action_ = new QAction(actionParent_);
  if (actionParent_) {
    actionParent_->addAction(shortcutDescriptionTable_[index].action_);
  }
  shortcutDescriptionTable_[index].action_->setShortcut(getShortcutText(index));
  if (actionSkin != QString::null)
    shortcutDescriptionTable_[index].action_->setIcon(QPixmap(getPath(RESOURCE) + "pic/" + actionSkin + ".png"));
}


FQTermShortcutHelper::ShortcutDescriptionEntry::ShortcutDescriptionEntry(const QString& key /*= QString("")*/,
                                                                         const QString& defaultshortcuttext /*= QString("")*/,
                                                                         const QString& description /*= QString("")*/) : 
                                                                          key_(key),
                                                                          defaultshortcuttext_(defaultshortcuttext),
                                                                          description_(description),
                                                                          action_(NULL) {}

FQTermShortcutHelper::ShortcutDescriptionEntry::~ShortcutDescriptionEntry() {
  delete action_;
}
}//namespace FQTerm

#include "fqterm_shortcuthelper.moc"
