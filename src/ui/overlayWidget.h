// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERMOVERLAYWIDGET_H
#define FQTERMOVERLAYWIDGET_H

#include <QFrame>
#include <QResizeEvent>
#include <QEvent>
#include <QHBoxLayout>

namespace FQTerm {
class OverlayWidget: public QFrame {
 public:
  /**
   * The widget is parented to the toplevelwidget of alignWidget,
   * this could be an issue if that widget has an autoAdd Layout
   */
  OverlayWidget(QWidget *parent, QWidget *anchor);
  virtual void reposition();

 protected:
  virtual void resizeEvent(QResizeEvent*);
  virtual bool eventFilter(QObject *, QEvent*);
  virtual bool event(QEvent*);

 private:
  QWidget *anchor_;
};

}  // namespace FQTerm

#endif  //  FQTERMOVERLAYWIDGET_H
