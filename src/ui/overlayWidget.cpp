// SPDX-License-Identifier: GPL-2.0-or-later

#include <QPoint>
#include <QFrame>
#include <QResizeEvent>
#include <QEvent>

#include "overlayWidget.h"

namespace FQTerm {

OverlayWidget::OverlayWidget(QWidget *parent, QWidget *anchor)
  : QFrame(parent->parentWidget()->parentWidget()),
    anchor_(anchor),
    parent_(parent) {
  parent->installEventFilter(this);

  hide();
}

void OverlayWidget::reposition() {
  setMaximumSize(parentWidget()->size());
  adjustSize();

  // p is in the alignWidget's coordinates
  QPoint p;

  p.setX(anchor_->width() - width());
  p.setY(-height());

  // Position in the toplevelwidget's coordinates
  QPoint pTopLevel = anchor_->mapTo(topLevelWidget(), p);

  // Position in the widget's parentWidget coordinates
  QPoint pParent = parentWidget()->mapFrom(topLevelWidget(), pTopLevel);
  // keep it on the screen
  if (pParent.x() < 0) {
    pParent.rx() = 0;
  }

  // Move 'this' to that position.
  move(pParent);
}

bool OverlayWidget::eventFilter(QObject *o, QEvent *e) {
  if (e->type() == QEvent::Move || e->type() == QEvent::Resize) {
    reposition();
  }

  return QFrame::eventFilter(o, e);
}

void OverlayWidget::resizeEvent(QResizeEvent *ev) {
  reposition();
  QFrame::resizeEvent(ev);
}

bool OverlayWidget::event(QEvent *e) {
  if (e->type() == QEvent::ChildAdded) {
    adjustSize();
  }

  return QFrame::event(e);
}

}
