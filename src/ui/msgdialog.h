// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_MSGDIALOG_H
#define FQTERM_MSGDIALOG_H

#include "ui_msgdialog.h"

class msgDialog: public QDialog {
  Q_OBJECT; 
public:
  msgDialog(QWidget *parent = 0, Qt::WindowFlags fl = 0);
  ~msgDialog();
  void setMessageText(const QString& message);
  Ui::msgDialog ui_;
};

#endif  // FQTERM_MSGDIALOG_H
