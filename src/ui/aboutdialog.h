// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_ABOUT_DIALOG_H
#define FQTERM_ABOUT_DIALOG_H

#include "ui_aboutdialog.h"

namespace FQTerm {

class aboutDialog: public QDialog {
  Q_OBJECT;
 public:
  aboutDialog(QWidget *parent = 0, Qt::WindowFlags fl = 0);
  ~aboutDialog();
 private:
  Ui::aboutDialog ui_;
};

}  // namespace FQTerm

#endif  // FQTERM_ABOUT_DIALOG_H
