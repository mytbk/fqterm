// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef __FQTERM_SHORTCUTHELPER__
#define __FQTERM_SHORTCUTHELPER__

#include "fqterm.h"
#include <QObject>
#include <QString>
#include <QMap>

class QWidget;
class QAction;
namespace FQTerm
{
class FQTermConfig;


#define FQTERM_ADDACTION(WIDGET, NAME, RECEIVER, FUNCTION) \
  do{ \
  QAction *action = getAction((FQTermShortcutHelper::NAME));\
  QObject::disconnect(action, SIGNAL(triggered()), (RECEIVER), SLOT(FUNCTION())); \
  FQ_VERIFY(connect(action, SIGNAL(triggered()), (RECEIVER), SLOT(FUNCTION()))); \
  (WIDGET)->addAction(action);\
  } while(0)



class FQTermShortcutHelper : public QObject
{
  Q_OBJECT;
public:
  enum FQTERM_SHORTCUT
  {
    FQTERM_APPLICATION_SHORTCUT_START = 0,
    CONNECT,
    DISCONNECT,
    CONN_INFO,
    ADDRESSBOOK,  //F2
    QUICKLOGIN,   //F3
    COPY,         //Ctrl+Ins under lin & win, Ctrl+C under mac
    PASTE,        //Shift+Insert under lin & win, Ctrl+V under mac
    COPYWITHCOLOR, 
    RECTANGLESELECTION,
    AUTOCOPYSELECTION,
    PASTEWORDWRAP,
    ENGLISHFONT,
    OTHERFONT,
    COLORSETTING,
    ANSICOLOR,
    REFRESHSCREEN,  //F5
    UIFONT,
    FULLSCREEN,   //F6
    BOSSCOLOR,    //F12
    SWITCHBAR,
    SEARCHIT,       //Ctrl+Alt+G
    WEIBOSHARE,
    EXTERNALEDITOR,   //Ctrl+Alt+E
    FASTPOST,   //Ctrl+Alt+F
    CURRENTSETTING,
    DEFAULTSETTING,
    SAVESETTING,
    PREFERENCE,
    SHORTCUTSETTING,
    EDITSCHEMA,
    COPYARTICLE,  //F9
    LOGRAW,
    ANTIIDLE, 
    AUTOREPLY,
    VIEWMESSAGE,  //F10
    IPLOOKUP,
    BEEP,
    MOUSESUPPORT,
    IMAGEVIEWER,
    RUNSCRIPT,    //F7
    STOPSCRIPT,   //F8
    RUNPYTHONSCRIPT, //Ctrl+F1
    ABOUT,        //F1
    HOMEPAGE,
    CASCADEWINDOWS,
    TILEWINDOWS,
    EXIT,
    COLORCTL_NO,
    COLORCTL_SMTH,
    COLORCTL_PTT,
    COLORCTL_OLD_CUSTOM,
    COLORCTL_CUSTOM,
    AUTORECONNECT,
    SCROLLBAR_LEFT,
    SCROLLBAR_RIGHT,
    SCROLLBAR_HIDDEN,
    SEARCH_GOOGLE,
    SEARCH_BAIDU,
    SEARCH_BING,
    SEARCH_YAHOO,
    SEARCH_CUSTOM,
    LANGUAGE_ENGLISH,
    NEXTWINDOW,
    PREVWINDOW,
    GLOBAL_SHOW_FQTERM,
    FQTERM_APPLICATION_SHORTCUT_END,
  };


public:
  FQTermShortcutHelper(FQTermConfig* config, QWidget* actionParent);
  ~FQTermShortcutHelper();
  
private:
  struct ShortcutDescriptionEntry
  {
    ShortcutDescriptionEntry(const QString& key = QString(""), const QString& defaultshortcuttext = QString(""), const QString& description = QString(""));
    ~ShortcutDescriptionEntry();
    QString key_;
    QString defaultshortcuttext_;
    QString description_;
    QAction* action_;
  };
  void initShortcutDescriptionTable();
  FQTermConfig* config_;
  QWidget* actionParent_;
  QMap<int, ShortcutDescriptionEntry> shortcutDescriptionTable_;
  void initShortcutDescriptionTableEntry(int index, const QString& key, const QString& defaultshortcuttext, const QString& description, const QString& actionSkin = QString());
  
public:

  QString getShortcutText(int shortcut) ;
  QString getShortcutDescription(int shortcut) {
    return shortcutDescriptionTable_[shortcut].description_;
  }
  QString getShortcutDefaultText(int shortcut) {
    return shortcutDescriptionTable_[shortcut].defaultshortcuttext_;
  }
  void setShortcutText(int shortcut, const QString& text);
  void resetShortcutText(int shortcut);
  void resetAllShortcutText();

  QAction* getAction(int shortcut) {
    return shortcutDescriptionTable_[shortcut].action_;
  }
  void retranslateActions();
private:
  //These functions are used to save shortcut
  QString getShortcutSection() {
    return QString("shortcutsettings");
  }
  QString getShortcutKey(int shortcut) {
    return shortcutDescriptionTable_[shortcut].key_;
  }
  
  QString getShortcutConfig(int shortcut);
  void setShortcutConfig(int shortcut, const QString& text);
  void retranslateAction(int shortcut, const QString& text);
};
  
  
}//namespace FQTerm


#endif 
