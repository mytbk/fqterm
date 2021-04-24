// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_TIME_LABEL_H
#define FQTERM_TIME_LABEL_H

#include <QTimerEvent>
#include <QLabel>

class QTime;

namespace FQTerm {

class FQTermTimeLabel: public QLabel {
  Q_OBJECT;
 public:
  FQTermTimeLabel(QWidget *parent, const char *name = 0, Qt::WindowFlags f = 0);
  ~FQTermTimeLabel();

 protected:
  QTime *currentTime_;
  void timerEvent(QTimerEvent*);
};

}  // namespace FQTerm

#endif  // FQTERM_TIME_LABEL_H
