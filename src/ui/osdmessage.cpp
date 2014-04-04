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

#include <QPaintEvent>
#include <QCursor>
#include <QTimer>
#include <QApplication>
#include <QPainter>
#include <QBitmap>
#include <QPalette>

#include "fqterm_trace.h"
#include "fqterm_path.h"

#include "osdmessage.h"

namespace FQTerm {

PageViewMessage::PageViewMessage(QWidget *parent_)
    : QWidget(parent_),
      timer_(0),
      message_() {
  setFocusPolicy(Qt::NoFocus);
  //setBackgroundMode( Qt::NoBackground );
  QPalette palette;
  palette.setColor(backgroundRole(), qApp->palette().color(QPalette::Active,
                                                           QPalette::Background));
  setPalette(palette);
  //     setPaletteBackgroundColor(qApp->palette().color(QPalette::Active, QPalette::Background));
  setCursor(QCursor(Qt::ArrowCursor));
  move(10, 10);
  resize(0, 0);
  hide();
}

// give to Caesar what Caesar owns: code taken from Amarok's osd.h/.cpp
// "redde (reddite, pl.) cesari quae sunt cesaris", just btw. ;)
void PageViewMessage::display(const QString &message, Icon icon, int durationMs, QPoint pos, PageViewMessage::Alignment ali)
{
  displayImpl(message, icon, false, durationMs, pos, ali);
}

void PageViewMessage::display(const QString &message, QPoint pos, Icon icon)
{
  displayImpl(message, icon, false, 4000, pos, TopLeft);
}

QRect PageViewMessage::displayCheck(const QString &message, Icon icon)
{
  return displayImpl(message, icon, true);
}

QRect PageViewMessage::displayImpl(const QString &message, Icon icon, bool check, int durationMs, QPoint pos, Alignment ali)
{
  //FIXME: add a option in fqterm too.
  /*
  if ( !KpdfSettings::showOSD() )
  {
  hide();
  return;
  }
  */
  // determine text rectangle

  if (!isHidden() && message == message_) {
    timer_->setSingleShot(true);
    timer_->start(durationMs);
    return QRect(QPoint(0, 0), size());
  }
  message_ = message;
  QRect textRect = fontMetrics().boundingRect(message);
  textRect.translate(-textRect.left(), -textRect.top());
  textRect.adjust(0, 0, 2, 2);
  int width = textRect.width(), height = textRect.height(), textXOffset = 0,
    shadowOffset = message.isRightToLeft() ? -1: 1;

  // load icon (if set) and update geometry
  // [FQTerm], we don't have a icon at this time.

  QPixmap symbol;
  if (icon != None) {
    switch (icon) {
      //case Find:
      //symbol = SmallIcon( "viewmag" );
      //break;
      case Error:
        symbol = QPixmap(getPath(RESOURCE) + "pic/messagebox_critical.png");
        break;
      case Warning:
        symbol = QPixmap(getPath(RESOURCE) + "pic/messagebox_warning.png");
        break;
      default:
        symbol = QPixmap(getPath(RESOURCE) + "pic/messagebox_info.png");
        break;
    }
    textXOffset = 2+symbol.width();
    width += textXOffset;
    height = qMax(height, symbol.height());
  }

  QRect geometry(0, 0, width + 10, height + 8);

  if (check)
    return geometry;

  // resize pixmap, mask and widget
  static QPixmap mask;
  mask = QPixmap(geometry.size());
  pixmap_ = QPixmap(geometry.size());
  resize(geometry.size());

  switch(ali)
  {
  case TopLeft:
    move(pos);
    break;
  case TopRight:
    move(pos - geometry.topRight());
    break;
  case BottomLeft:
    move(pos - geometry.bottomLeft());
    break;
  case BottomRight:
    move(pos - geometry.bottomRight());
    break;
  }

  // create and set transparency mask
  QPainter maskPainter(&mask);
  mask.fill(Qt::black);
  maskPainter.setBrush(Qt::white);
  maskPainter.drawRoundRect(geometry, 1600 / geometry.width(), 1600 /
    geometry.height());
  setMask(mask.createHeuristicMask());

  // draw background
  QPainter bufferPainter(&pixmap_);
  bufferPainter.setPen(Qt::black);
  bufferPainter.setBrush(palette().brush(QPalette::Window));
  bufferPainter.drawRoundRect(geometry, 1600 / geometry.width(), 1600 /
    geometry.height());

  // draw icon if present
  if (!symbol.isNull()) {
    bufferPainter.drawPixmap(5, 4, symbol, 0, 0, symbol.width(), symbol.height());
  }

  // draw shadow and text
  int yText = geometry.height() - height / 2;
  bufferPainter.setPen(palette().color(QPalette::Window).dark(115));
  bufferPainter.drawText(5+textXOffset + shadowOffset, yText + 1, message);
  bufferPainter.setPen(palette().color(QPalette::WindowText));
  bufferPainter.drawText(5+textXOffset, yText, message);

  // show widget and schedule a repaint
  show();
  update();

  // close the message window after given mS
  if (durationMs > 0) {
    if (!timer_) {
      timer_ = new QTimer(this);
      timer_->setSingleShot(true);
      FQ_VERIFY(connect(timer_, SIGNAL(timeout()), SLOT(hide())));
    }
    timer_->start(durationMs);
  } else if (timer_) {
    timer_->stop();
  }

  return QRect(QPoint(0, 0), size());
}

void PageViewMessage::paintEvent(QPaintEvent *e) {
  QPainter p(this);
  p.drawPixmap(e->rect().topLeft(), pixmap_, e->rect());
}

void PageViewMessage::mousePressEvent(QMouseEvent * /*e*/) {
  if (timer_) {
    timer_->stop();
  }
  hide();
}

void PageViewMessage::hideEvent( QHideEvent * event ) {
  emit hideAt(geometry ());
  QWidget::hideEvent(event);
}

}  // namespace FQTerm

#include "osdmessage.moc"
