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

#include "popwidget.h"

#include <QPixmap>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QLabel>

#include <QMouseEvent>
#include <QPalette>

namespace FQTerm {

extern QString pathLib;

popWidget::popWidget(FQTermWindow *win, QWidget *parent_, const char *name,
                     Qt::WindowFlags f)
    : QWidget(parent_, f) {
  //     QPixmap pxm(QPixmap(pathLib+"pic/popwidget.png") );
  //     if(!pxm.isNull())
  //     {
  //     	resize(pxm.width(), pxm.height());
  //     	QPalette palette;
  //     	palette.setBrush(backgroundRole(), QBrush(pxm));
  //     	setPalette(palette);
  //     	//setBackgroundPixmap(pxm);
  //     }
  //     else
  //     {
  //     	resize(200, 120);
  //     }
  //
  //     label_ = new QLabel(this);
  //     label_->setGeometry( QRect( 5, height()/3, width()-10, height()*2/3 ) );
  //     label_->setAlignment( Qt::AlignTop );
  //     label_->setWordWrap(true);
  //     if(!pxm.isNull()){
  //     	QPalette palette;
  //     	palette.setBrush(label_->backgroundRole(), QBrush(pxm));
  //     	label_->setPalette(palette);
  //     //	label_->setBackgroundPixmap(pxm);
  //     }
  //     else {
  //     	QPalette palette;
  //     	palette.setColor(label_->backgroundRole(), QColor(249,250,229));
  //     	label_->setPalette(palette);
  //     	//label_->setBackgroundColor(QColor(249,250,229));
  //     }
  //     //label_->setBackgroundOrigin( ParentOrigin );
  //     label_->setFont(QFont(qApp->font().family(), 12));
  //
  //     pTimer = new QTimer(this);
  //     QVERIFY(connect(pTimer, SIGNAL(timeout()), this, SLOT(showTimer())));
  //
  //     stateID = -1;
  //
  //     stepID_ = 2;
  //
  //     intervalID_ = 500/(height()/stepID_);
  //
  //     setFocusPolicy(Qt::NoFocus);
  //
  //     termWindow_ = win;
  //
  //     hide();
  //
  //     Display *dsp = QX11Info::display();
  //     int screen = DefaultScreen(dsp);
  //    Window root = RootWindow(dsp, screen);
  //
  //     WId winid = this->winId();
  //
  //     XEvent ev;
  //     memset(&ev, 0, sizeof(ev));
  //
  //     ev.xclient.type = ClientMessage;
  //     ev.xclient.termWindow_ = winid;
  //     ev.xclient.message_type = XInternAtom (dsp, "_NET_WM_DESKTOP", False );
  //     ev.xclient.format = 32;
  //     ev.xclient.data.l[0] = -1;
  //     ev.xclient.data.l[1] = 0l;
  //     ev.xclient.data.l[2] = 0l;
  //     ev.xclient.data.l[3] = 0l;
  //     ev.xclient.data.l[4] = 0l;
  //
  //     XSendEvent(dsp, root, False, SubstructureRedirectMask|SubstructureNotifyMask, &ev);
  //     XSync(dsp, False);
}

popWidget::~popWidget() {
  //	delete pTimer;
}

void popWidget::mousePressEvent(QMouseEvent *me) {
  //    QWidgetList list = QApplication::topLevelWidgets();
  //
  //    //((FQTermFrame *)qApp->mainWidget())->popupFocusIn(termWindow_);
  //
  //    if(stateID==1)
  //    {
  //    	stateID = 2;
  //    	pTimer->setInterval(intervalID_);
  //    }
}


void popWidget::popup() {
  //    pTimer->start(intervalID_);
  //    stateID = 0;
  //
  //    desktopRectangle_ = qApp->desktop()->rect();
  //    position_ = QPoint( desktopRectangle_.width()-width()-5, desktopRectangle_.height()-5 );
  //    move(position_);
  //
  //    if(!isVisible())
  //    	show();
}

void popWidget::setText(const QString &str) {
  //	label_->setText(str);
}

void popWidget::showTimer() {
  //    switch(stateID)
  //    {
  //    case 0:		// popup
  //    	if(position_.y()+height()+5>desktopRectangle_.height())
  //    		position_.setY( position_.y() - stepID_ );
  //    	else
  //    	{
  //    		stateID = 1;
  //    		pTimer->setInterval(5000);
  //    	}
  //    	break;
  //    case 1:		// wait
  //    	stateID = 2;
  //    	pTimer->setInterval(intervalID_);
  //    	break;
  //    case 2:		// hiding
  //    	if(position_.y()<desktopRectangle_.height())
  //    		position_.setY( position_.y() + stepID_ );
  //    	else
  //    	{
  //    		stateID = -1;
  //    		pTimer->stop();
  //    	}
  //    	break;
  //    default:
  //
  //    	break;
  //    }
  //    move(position_);
}

}  // namespace FQTerm

#include "popwidget.moc"
