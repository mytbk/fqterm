// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SSH_LOGIN_H
#define FQTERM_SSH_LOGIN_H

#include "ui_sshlogin.h"

namespace FQTerm {

class SSHLoginDialog: public QDialog {
  Q_OBJECT; 
public:
  SSHLoginDialog(QString *username, QString *password,
                 QWidget *parent_ = 0, Qt::WindowFlags fl = 0);
  ~SSHLoginDialog();
private:
  QString *strUserName;
  QString *strPassword;
  Ui::SSHLogin ui_;
private slots:
  void accept();
  void moveFocus();
};

}  // namespace FQTerm

#endif  // FQTERM_SSH_LOGIN_H
