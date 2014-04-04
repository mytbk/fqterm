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

/////////////////////////////////////////////////////////////////
//TODO: This class has too many overlap with MDI Area.
//It should inherit the QMdiArea...
/////////////////////////////////////////////////////////////////

#include <stdio.h>

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QKeySequence>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QTabBar>
#include <QMouseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QToolButton>
#include "fqterm_frame.h"
#include "fqterm_path.h"
#include "fqterm_screen.h"
#include "fqterm_trace.h"
#include "fqterm_window.h"
#include "fqterm_wndmgr.h"
#ifdef HAVE_PYTHON
#include <Python.h>
#endif //HAVE_PYTHON
namespace FQTerm {

//constructor
FQTermWndMgr::FQTermWndMgr(QWidget *parent, const char *name)
    : QMdiArea(parent),
      subWindowMax_(true),
      subWindowSize_(640, 480),
      tabBar_(NULL)
 {
  tabBar_ = new FQTermTabBar(this);
  tabBar_->setShape(QTabBar::RoundedSouth);
  FQ_VERIFY(connect(tabBar_, SIGNAL(currentChanged(int)),
    this, SLOT(activateTheWindow(int))));
  FQ_VERIFY(connect(tabBar_, SIGNAL(rightClicked(int,const QPoint&)),
    this, SLOT(onTabRightClicked(int,const QPoint&))));
  FQ_VERIFY(connect(tabBar_, SIGNAL(doubleClicked(int,const QPoint&)),
    this, SLOT(onTabDoubleClicked(int,const QPoint&))));
  setObjectName(name);
  termFrame_ = (FQTermFrame*)parent;
  isAfterRemoved_ = false;
}

//destructor
FQTermWndMgr::~FQTermWndMgr(){}


//remove window-tab-iconset
bool FQTermWndMgr::closeWindow(FQTermWindow *mw) {
  FQ_FUNC_TRACE("wndmgr", 3);
  QMdiSubWindow* subWindow = FQToMDI(mw);
  if (subWindow) {
    return subWindow->close();
  }
  return false;
}

bool FQTermWndMgr::closeAllWindow() {
  FQ_FUNC_TRACE("wndmgr", 3);

  bool anyConnected = false;

  for (int i = 0; i < subWindowList().size(); ++i) {
    FQTermWindow * window = (FQTermWindow *)(subWindowList()[i]->widget());
    if (window->isConnected()) {
      anyConnected = true;
      break;
    }
  }

  bool warn = FQTermPref::getInstance()->openWarnOnClose_;
  if (anyConnected && warn) {
    QMessageBox mb(tr("FQTerm"),
      tr("Still connected, do you really want to exit?"),
      QMessageBox::Warning, QMessageBox::Yes|QMessageBox::Default,
      QMessageBox::No | QMessageBox::Escape, 0, this);
    if (mb.exec() != QMessageBox::Yes) {
      return false;
    }
  }
  FQTermPref::getInstance()->openWarnOnClose_ = false;
  while (count() > 0) {
    bool closed = subWindowList().at(0)->close();
    if (!closed) {
      FQTermPref::getInstance()->openWarnOnClose_ = warn;
      return false;
    }
  }
  FQTermPref::getInstance()->openWarnOnClose_ = warn;
  return true;
}



//active the window when switch the tab
void FQTermWndMgr::activateTheWindow(int n) {
  FQ_FUNC_TRACE("wndmgr", 3);

  if (n < 0 || n >= count() || subWindowList().at(n) == activeSubWindow())
    return;

  QMdiSubWindow *subWindow = subWindowList().at(n);

  // Fix the refresh bug by send PaintEvent to session screen.
  //subWindow->setFocus();
  setActiveSubWindow(subWindow);
  
}

//blink the tab when message come in
void FQTermWndMgr::blinkTheTab(FQTermWindow *mw, bool bVisible) {
  FQ_FUNC_TRACE("wndmgr", 10);
  static QIcon a_icon(getPath(RESOURCE) + "pic/tabpad.png");
  static QIcon b_icon(getPath(RESOURCE) + "pic/transp.png");

  //find where it is
  QMdiSubWindow* subWindow = FQToMDI(mw);
  if (!subWindow)
    return;
  int n = subWindowList().indexOf(subWindow);
  QIcon *icon = icons_.at(n);
  //FIXME: QIcon::Automatic
  if (bVisible) {
    icon->addFile(getPath(RESOURCE) + "pic/tabpad.png");

    tabBar_->setTabIcon(n, a_icon);
  } else {
    //,QIcon::Automatic);
    icon->addFile(getPath(RESOURCE) + "pic/transp.png");
    tabBar_->setTabIcon(n, b_icon);
  }
  //,QIcon::Automatic);


  

  tabBar_->update();
}

//return the number of connected window
int FQTermWndMgr::count() {
  return subWindowList().count();
}

FQTermWindow *FQTermWndMgr::activeWindow() {
  return MDIToFQ(activeSubWindow());
}


bool FQTermWndMgr::afterRemove() {
  if (isAfterRemoved_) {
    isAfterRemoved_ = false;
    return true;
  } else {
    return false;
  }
}

void FQTermWndMgr::refreshAllExcept(FQTermWindow *termWindow) {
  for (int i = 0; i < subWindowList().count(); ++i) {
    FQTermWindow * window = (FQTermWindow *)(subWindowList()[i]->widget());
    if (window != termWindow) {
      window->repaintScreen();
    }
  }
}


FQTermWindow* FQTermWndMgr::newWindow( const FQTermParam &param, FQTermConfig* config, QIcon* icon, int index /*= -1*/ )
{
  FQTermWindow *window = new FQTermWindow(config, termFrame_, param, index, this, 0);
  //add window-tab-icon to window manager


  QMdiSubWindow* subWindow = addSubWindow(window);
  subWindow->setAttribute(Qt::WA_OpaquePaintEvent);
  subWindow->setAttribute(Qt::WA_DeleteOnClose);

  icons_.append(icon);
  if (count() == 1) {
    termFrame_->enableMenuToolBar(true);
  }
  //if no this call, the tab wont display untill you resize the window
  int idx = tabBar_->addTab(*icon, window->windowTitle());
  tabBar_->updateGeometry();
  tabBar_->update();

  subWindow->resize(subWindowSize_);
  if (subWindowMax_) {
    subWindow->setWindowFlags(Qt::SubWindow | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);
    subWindow->showMaximized();
  } else {
    subWindow->setWindowFlags(Qt::SubWindow | (Qt::CustomizeWindowHint
      | Qt::WindowMinMaxButtonsHint
      | Qt::WindowSystemMenuHint));
    subWindow->show();
  }
  tabBar_->setTabData(idx, qVariantFromValue((QObject*)window));
  FQ_VERIFY(connect(window, SIGNAL(destroyed(QObject*)), 
    this ,SLOT(onSubWindowClosed(QObject*))));
  FQ_VERIFY(connect(window, SIGNAL(resizeSignal(FQTermWindow*)),
		    this, SLOT(subWindowResized(FQTermWindow*))));
  FQ_VERIFY(connect(this, SIGNAL(subWindowActivated(QMdiSubWindow*)),
        this, SLOT(onSubWindowActivated(QMdiSubWindow*))));
  FQ_VERIFY(connect(window, SIGNAL(blinkTheTab(FQTermWindow*, bool)),
       this, SLOT(blinkTheTab(FQTermWindow*, bool))));
  FQ_VERIFY(connect(window, SIGNAL(refreshOthers(FQTermWindow*)),
       this, SLOT(refreshAllExcept(FQTermWindow*))));
  FQ_VERIFY(connect(window, SIGNAL(connectionClosed(FQTermWindow*)),
    this, SLOT(closeWindow(FQTermWindow*)), Qt::QueuedConnection));
  return window;
}

void FQTermWndMgr::onSubWindowClosed(QObject* obj) {

  for (int i = 0; i < tabBar_->count(); ++i) {
    if (tabBar_->tabData(i) == qVariantFromValue(obj)) {
      if (count() == 0) {
        termFrame_->enableMenuToolBar(false);
      }
      if (i == tabBar_->currentIndex())
        isAfterRemoved_ = true;
      tabBar_->removeTab(i);
      icons_.removeAt(i);
    }
  }

}

//---------------------------
//record subwindows' size 
//---------------------------
void
FQTermWndMgr::subWindowResized(FQTermWindow *mw) {
  QMdiSubWindow* subWindow = FQToMDI(mw);
  if (!subWindow)
    return;
  Qt::WindowFlags wfs = subWindow->windowFlags();

  if (!(subWindowMax_ = subWindow->isMaximized())){
    if (!subWindow->isMinimized()) {
      subWindowSize_ = subWindow->frameSize();
    }
    subWindow->setWindowFlags(wfs | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
  }
  else {
    subWindow->setWindowFlags(wfs & ~Qt::WindowSystemMenuHint);
  }

}

void FQTermWndMgr::cascade() {
  QSize oldSize = subWindowSize_;
  cascadeSubWindows();
  foreach(QMdiSubWindow* subWindow, subWindowList()) {
    if (subWindow) {
      subWindow->resize(oldSize);
    }
  }
}

void FQTermWndMgr::tile() {
  tileSubWindows();
}

void FQTermWndMgr::activateNextWindow() {
  activateTheWindow((activeWindowIndex() + 1) % count());
}

void FQTermWndMgr::activatePrevWindow() {
  activateTheWindow((activeWindowIndex() - 1 + count()) % count());
}

FQTermWindow * FQTermWndMgr::nthWindow(int n) {
  return (FQTermWindow *)(subWindowList()[n]->widget());
}

void FQTermWndMgr::onSubWindowActivated(QMdiSubWindow * subWindow) {
  int n = subWindowList().indexOf(subWindow);
  if (n == -1)
    return;
  FQTermWindow* mw = MDIToFQ(subWindow);
  if (mw->isMaximized() &&
    afterRemove()) {
      mw->showNormal();
      mw->showMaximized();
  }
  if (tabBar_->currentIndex() != n) {
    FQ_VERIFY(disconnect(this, SIGNAL(subWindowActivated(QMdiSubWindow*)),
      this, SLOT(onSubWindowActivated(QMdiSubWindow*))));
    tabBar_->setCurrentIndex(n);
    FQ_VERIFY(connect(this, SIGNAL(subWindowActivated(QMdiSubWindow*)),
      this, SLOT(onSubWindowActivated(QMdiSubWindow*))));
  }
  termFrame_->updateMenuToolBar();
}

void FQTermWndMgr::onTabRightClicked(int index, const QPoint& p) {
  QMenu* menu = FQToMDI(nthWindow(index))->systemMenu();
  menu->popup(p);
}

void FQTermWndMgr::onTabDoubleClicked(int index, const QPoint& p) {
  closeWindow(nthWindow(index));
}

FQTermWindow* FQTermWndMgr::MDIToFQ( QMdiSubWindow* subWindow )
{
  if (!subWindowList().contains(subWindow))
    return NULL;
  return (FQTermWindow*)subWindow->widget();
}

QMdiSubWindow* FQTermWndMgr::FQToMDI( FQTermWindow* window )
{
  for (int i = 0; i < subWindowList().count(); ++i) {
    if ((FQTermWindow*)(subWindowList()[i]->widget()) == window) {
      return subWindowList()[i];
    }
  } 
  return NULL;
}
 
int FQTermWndMgr::FQToIndex( FQTermWindow* window )
{
  return subWindowList().indexOf(FQToMDI(window));
}

bool FQTermWndMgr::event( QEvent* e ) {
  if (e->type() == QEvent::ShortcutOverride) {
    QKeyEvent* ke = (QKeyEvent*)e;
    if (ke->key() == Qt::Key_W && ke->modifiers() == Qt::ControlModifier) {
      ke->accept();
      return true;
    }
  }
  return QMdiArea::event(e);
}
void FQTermTabBar::mouseReleaseEvent( QMouseEvent * me ) {
  int index = tabAt(me->pos());
  if (index != -1 && me->button() == Qt::RightButton && me->modifiers() == Qt::NoModifier) {
    emit rightClicked(index, mapToGlobal(me->pos()));
  }
}

void FQTermTabBar::mouseDoubleClickEvent( QMouseEvent * me ) {
  int index = tabAt(me->pos());
  if (index != -1 && me->button() == Qt::LeftButton && me->modifiers() == Qt::NoModifier) {
    emit doubleClicked(index, mapToGlobal(me->pos()));
  }
}
}  // namespace FQTerm

#include "fqterm_wndmgr.moc"
