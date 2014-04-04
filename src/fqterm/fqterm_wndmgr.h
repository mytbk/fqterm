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

#ifndef FQTERM_WND_MGR_H
#define FQTERM_WND_MGR_H

#include <QString>
#include <QList>
#include <map>
#include <utility>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QTabBar>

class QEvent;
class QIcon;
class QSize;
class FQTermConfig;
namespace FQTerm {

class FQTermWindow;
class FQTermFrame;

class FQTermTabBar : public QTabBar {
  Q_OBJECT;
public:
  FQTermTabBar(QWidget *parent = 0) : QTabBar(parent) {}
protected:
  virtual void mouseReleaseEvent(QMouseEvent * me);
  virtual void mouseDoubleClickEvent(QMouseEvent * me);
signals:
  void rightClicked(int index, const QPoint& p);
  void doubleClicked(int index, const QPoint& p);
};

class FQTermWndMgr: public QMdiArea {
  Q_OBJECT;
 public:
  FQTermWndMgr(QWidget *parent = 0, const char *name = 0);
  ~FQTermWndMgr();

  QTabBar* tabBar() {return tabBar_;}
  
  FQTermWindow* newWindow(const FQTermParam &param, FQTermConfig* config, QIcon* icon, int index = -1);
  bool closeAllWindow();



  FQTermWindow *activeWindow();
  int activeWindowIndex() {
    return FQToIndex(activeWindow());
  }
  FQTermWindow *nthWindow(int n);
  int count();

  //sub-window position & size

  bool getSubWindowMax() const { return subWindowMax_; }
  void setSubWindowMax(bool val) { subWindowMax_ = val; }
  QSize getSubWindowSize() const { return subWindowSize_; }
  void setSubWindowSize(QSize val) { subWindowSize_ = val; }

 public slots:
   bool closeWindow(FQTermWindow *mw);
   void onSubWindowClosed(QObject* obj);
   //record subwindows' size changes
   void subWindowResized(FQTermWindow *);
   void activateTheWindow(int n);
   void activateNextWindow();
   void activatePrevWindow();
   void refreshAllExcept(FQTermWindow *mw);
   void blinkTheTab(FQTermWindow *mw, bool bVisible);
   void cascade();
   void tile();

protected:

  QList<QIcon *> icons_;
  FQTermFrame *termFrame_;
  bool isAfterRemoved_;
  QSize subWindowSize_;
  bool subWindowMax_;
  FQTermTabBar* tabBar_;

  FQTermWindow* MDIToFQ(QMdiSubWindow* subWindow);
  QMdiSubWindow* FQToMDI(FQTermWindow* window);
  int FQToIndex(FQTermWindow* window);
  bool afterRemove();
  bool event(QEvent* e);
protected slots:
  void onSubWindowActivated(QMdiSubWindow * subWindow);
  void onTabRightClicked(int index, const QPoint& p);
  void onTabDoubleClicked(int index, const QPoint& p);
};

}  // namespace FQTerm

#endif  // FQTERM_WND_MGR_H
