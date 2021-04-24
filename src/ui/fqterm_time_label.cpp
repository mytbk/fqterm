// SPDX-License-Identifier: GPL-2.0-or-later

#include "fqterm_time_label.h"

#include <QLabel>
#include <QTimerEvent>
#include <QDateTime>

namespace FQTerm {

//constructor
FQTermTimeLabel::FQTermTimeLabel(QWidget *parent, const char *name, Qt::WindowFlags f)
    : QLabel(parent, f) {
  setAlignment(Qt::AlignHCenter);
  currentTime_ = new QTime();
  currentTime_->start();
  startTimer(1000);
}

//destructor
FQTermTimeLabel::~FQTermTimeLabel() {
  delete currentTime_;
}

//timer to display the current time
void FQTermTimeLabel::timerEvent(QTimerEvent*) {
  setText(currentTime_->currentTime().toString());
}

}  // namespace FQTerm

#include "fqterm_time_label.moc"
