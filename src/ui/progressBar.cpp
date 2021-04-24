// SPDX-License-Identifier: GPL-2.0-or-later

#include <QLabel>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>

#include "fqterm_trace.h"
#include "fqterm_path.h"
#include "progressBar.h"

namespace FQTerm {

ProgressBar::ProgressBar(QWidget *parent, QLabel *label)
  : QProgressBar(parent),
    label_(label),
    isFinished_(false) {
  //DEBUG_FUNC_INFO
  setMaximum(100);
  label_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  abortButton_ = new QPushButton("Abort", parent);
  abortButton_->setObjectName("Abort");
  abortButton_->hide();
  //m_abort->setText( tr("Abort") );
  abortButton_->setIcon(QPixmap(getPath(RESOURCE) + "pic/messagebox_critical.png"));
  label_->show();
  show();
}

ProgressBar::~ProgressBar() {
  //DEBUG_FUNC_INFO
}

ProgressBar &ProgressBar::setDescription(const QString &text) {
  description_ = text;
  label_->setText(text);

  return  *this;
}

ProgressBar &ProgressBar::setStatus(const QString &text) {
  QString s = description_;
  s += " [";
  s += text;
  s += ']';

  label_->setText(s);
  parentWidget()->adjustSize();

  return  *this;
}

ProgressBar &ProgressBar::setAbortSlot(QObject *receiver, const char *slot) {
  FQ_VERIFY(connect(abortButton_, SIGNAL(clicked()), receiver, slot));
  FQ_VERIFY(connect(abortButton_, SIGNAL(clicked()), this, SLOT(hide())));
  abortButton_->show();

  parentWidget()->adjustSize();

  return  *this;
}

void ProgressBar::setDone() {
  if (!isFinished_) {
    isFinished_ = true;
    abortButton_->setEnabled(false);
    setStatus(tr("Done"));
  } else {
    // then we we're aborted
    setStatus(tr("Aborted"));
  }
}

void ProgressBar::hide() {
  //NOTE naughty

  isFinished_ = true;
  abortButton_->setEnabled(false);
  setStatus(tr("Aborting..."));
}

}  // namespace FQTerm
