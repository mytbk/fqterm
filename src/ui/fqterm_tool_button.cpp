// SPDX-License-Identifier: GPL-2.0-or-later

#include "fqterm_tool_button.h"
#include "fqterm_trace.h"

namespace FQTerm {

FQTermToolButton::FQTermToolButton(QWidget *parent_, int id, QString name)
    : QToolButton(parent_) {
  setObjectName(name);
  this->id_ = id;
  FQ_VERIFY(connect(this, SIGNAL(clicked()), this, SLOT(slotClicked())));
}

FQTermToolButton::~FQTermToolButton(){}

void FQTermToolButton::slotClicked() {
  emit(buttonClicked(id_));
}

}  // namespace FQTerm

#include "fqterm_tool_button.moc"
