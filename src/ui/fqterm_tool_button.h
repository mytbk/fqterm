// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_TOOL_BUTTON_H
#define FQTERM_TOOL_BUTTON_H

#include <QToolButton>

namespace FQTerm {

class FQTermToolButton: public QToolButton {
  Q_OBJECT;
 public:
  FQTermToolButton(QWidget *parent_, int id, QString name = "");
  ~FQTermToolButton();

 signals:
  void buttonClicked(int);

 protected slots:
  void slotClicked();

 protected:
  int id_;
};

}  // namespace FQTerm

#endif // FQTERM_TOOL_BUTTON_H
