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

#ifndef FQTERM_FRAME_H
#define FQTERM_FRAME_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenuBar>

#include <vector>

class QWidget;
class QLineEdit;
class QToolButton;
class QMdiArea;
class QSignalMapper;
class QTabBar;
class QFontDialog;
class QActionGroup;
class QString;
class QFontComboBox;
class UAOCodec;

#ifdef USE_GLOBAL_HOTKEY
class QxtGlobalShortcut;
#endif //USE_GLOBAL_HOTKEY

namespace FQTerm {

class IPLookupDialog;
class DefineEscapeDialog;
class FQTermImage;
class FQTermParam;
class FQTermConfig;
class FQTermWndMgr;
class FQTermWindow;
class StatusBar;
class FQTermTimeLabel;
class TranslatorInstaller;
class FQTermMiniServerThread;
class FQTermShortcutHelper;

#ifdef HAVE_PYTHON
class FQTermPythonHelper;
#endif //HAVE_PYTHON

class FQTermFrame: public QMainWindow {
  Q_OBJECT;
 public:
  FQTermFrame();
  ~FQTermFrame();

  void updateMenuToolBar();
  void enableMenuToolBar(bool);

  void popupFocusIn(FQTermWindow*);

  void viewImages(QString filename, bool raiseViewer);
  void buzz(FQTermWindow* window = NULL);
  void installTranslator(const QString& lang);
  FQTermConfig * config() const { return config_; }

  static const int translatedModule = 4;
  static const QString qmPrefix[translatedModule];
  static const QString qmPostfix;

 signals:
  void bossColor();
  void updateScroll();
  void changeLanguage();
  void fontAntiAliasing(bool);

 protected slots:
  
  bool event(QEvent *event);

  void viewImages();
  bool clearUp();

  // Menu
  void recreateMenu();
  void addressBook();
  void quickLogin();
  void exitFQTerm();

  void aboutFQTerm();
  void langEnglish();
  void defaultSetting();
  void preference();
  void shortcutSetting();
  void runScript();
  void delayedRunScript(); // to avoid activate recursion guard
  void stopScript();
  void runPyScript();
  void homepage();

  void toggleAnsiColor();

  // Toolbar
  void keyClicked(int);

  void connectIt();
  void disconnect();
  void copy();
  void paste();
  void searchIt();
  void shareIt();
  void externalEditor();
  void fastPost();
  void copyRect();
  void copyColor();
  void copyArticle();
  void autoCopy();
  void wordWrap();
  void noEsc();
  void escEsc();
  void uEsc();
  void oldCustomEsc();
  void customEsc();
  void hideScroll();
  void leftScroll();
  void rightScroll();
  void setSEGoogle();
  void setSEBaidu();
  void setSEYahoo();
  void setSEBing();
  void setSECustom();
  void showSwitchBar();
  void setFont();
  void setColor();
  void refreshScreen();
  void fullscreen();
  void bosscolor();
  void toggleServer(bool on);
  void uiFont();
  void antiIdle();
  void autoReply();
  void setting();
  void saveSessionSetting();
  void viewMessages();
  void enableMouse();
  void beep();
  void reconnect();
  void keySetup();
  void ipLookup();
  void termFontChange(bool isEnglish, QFont font);
  void comboFontChanged(const QFont & font);

  void themesMenuAboutToShow();
  void themesMenuActivated();
  void windowsMenuAboutToShow();
  void connectMenuActivated();
  void popupConnectMenu();
  void trayActived(QSystemTrayIcon::ActivationReason);
 
  //void trayClicked(const QPoint &, int);
  //void trayDoubleClicked();
  void trayHide();
  void trayShow();
  void buildTrayMenu();

  void reloadConfig();

  void saveSetting();
  void schemaUpdated();
  void editSchema();
 private:

  FQTermWndMgr *windowManager_;
  // image viewer
  FQTermImage *imageViewer_;

  FQTermTimeLabel *labelTime_;

  QString theme_;

  QActionGroup *escapeGroup;
  QActionGroup *languageGroup;
  QActionGroup *scrollGroup;
  QActionGroup *searchEngineGroup;
  QMenu *menuWindows_;
  QMenu *menuThemes_;
  QMenu *menuFont_;
  QMenu *menuFile_;
  QMenu *menuLanguage_;
  QMenu *menuConnect_;
  QSignalMapper* windowMapper_;

  FQTerm::StatusBar *statusBar_;

  QToolButton *serverButton_;
  QToolButton *connectButton_; 
  QToolButton *fontButton_;
  QFontComboBox *englishFontCombo_;
  QFontComboBox *otherFontCombo_;

  QMenuBar *menuMain_;
  QToolBar *toolBarMdiConnectTools_;
  QToolBar *toolBarMdiTools_;
  QToolBar *toolBarSetupKeys_;
  QToolBar *toolBarFonts_;

  bool isTabBarShown_;

  QSystemTrayIcon *tray_;

  QMenu *trayMenu_;

  QTranslator * translator[translatedModule];
  QList<TranslatorInstaller*> installerList_;

  FQTermConfig * config_;
  FQTermShortcutHelper * shortcutHelper_;
  QAction* getAction(int shortcut);
  FQTermMiniServerThread* serverThread_;

  UAOCodec* uaoCodec_;

private:
  void newWindow(const FQTermParam &param, int index = -1);

  void closeEvent(QCloseEvent*);
  void selectStyleMenu(int, int);
  void iniSetting();
  void loadPref();

  void addMainMenu();
  void addMainTool();

  void setSE(const QString& se);

  void updateKeyToolBar();
  void updateFontCombo();

  void loadToolBarPosition();

  bool eventFilter(QObject *, QEvent*);

  void insertThemeItem(const QString&);
  void setUseDock(bool);

  void initAdditionalActions();

  void initTranslator();
  void clearTranslator();
  void connector();
  void updateLanguageMenu();

  void loadStyleSheetFromFile(const QString qssFile);
  void refreshStyleSheet();
  void clearStyleSheet();

  bool isDelimiterExistedBefore(const QString& str,
                                const std::vector<QString>& existingDelimiters);
  int isDelimiterExistedAfter(const QString& str,
                              const std::vector<QString>& existingDelimiters);
  bool uppercaseCharFollowingCtrl(QString& str,
                                  int& i,
                                  const QString& after);
  void replaceEscapeString(QString& str,
                           const QString& before,
                           const QString& after,
                           const QString& delimiter,
                           const std::vector<QString> *existingDelimiters = NULL);
  void transEscapeStr(QString& target, const QString& source);

#ifdef HAVE_PYTHON
public:
  FQTermPythonHelper* getPythonHelper() {
    return pythonHelper_;
  }

  //protected slots:
private:
  FQTermPythonHelper* pythonHelper_;
#endif //HAVE_PYTHON

#ifdef USE_GLOBAL_HOTKEY
  QxtGlobalShortcut* globalHotkey_;
private slots:
  void globalHotkeyTriggered();
  void globalHotkeyChanged();
#endif //USE_GLOBAL_HOTKEY
};

class TranslatorInstaller : public QObject
{
  Q_OBJECT;

public:
  TranslatorInstaller(const QString& language, FQTermFrame* frame);

  QString languageName();
  QString languageFormalName();
public slots:
  void installTranslator();

protected:
  QString language_;
  FQTermFrame* frame_;

  QString languageName_;
};


}  // namespace FQTerm

#endif  // FQTERM_FRAME_H
