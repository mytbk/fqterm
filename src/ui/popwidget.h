// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_POP_WIDGET_H
#define FQTERM_POP_WIDGET_H

#include <QWidget>
//Added by qt3to4:
#include <QMouseEvent>
#include <QLabel>

class QTimer;
class QLabel;

namespace FQTerm {

class FQTermWindow;

class popWidget: public QWidget {
  Q_OBJECT;
 public:
  //	#if (QT_VERSION>=310)
  //	popWidget(FQTermWindow * win, QWidget *parent = 0, const char *name=0, WindowFlags f=WStyle_Splash);
  //	#else
  popWidget(FQTermWindow *win, QWidget *parent_ = 0, const char *name = 0,
            Qt::WindowFlags f = Qt::WindowStaysOnTopHint
            | Qt::X11BypassWindowManagerHint
            | Qt::Tool);
  //	#endif
  ~popWidget();

  void popup();
  void setText(const QString &);

 public slots:
  void showTimer();

 protected:
  QTimer *pTimer;
  int stateID; // -1 hide, 0 popup, 1 wait, 2 hiding
  QPoint position_;
  QRect desktopRectangle_;
  int stepID_;
  int intervalID_;
  QLabel *label_;
  FQTermWindow *termWindow_;

  void mousePressEvent(QMouseEvent*);
};

}  // namespace FQTerm

#endif  // FQTERM_POP_WIDGET_H
