/***************************************************************************
 *   fqterm, a terminal emulator for both BBS and *nix.                    *
 *   Copyright (C) 2008 fqterm development group.                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.               *
 ***************************************************************************/

#include <QString>
#include <QLineEdit>

#include "fqterm_trace.h"
#include "sshlogindialog.h"

namespace FQTerm {

/* 
 *  Constructs a fSSHLogin which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SSHLoginDialog::SSHLoginDialog(QString *username, QString *password,
                               QWidget *parent_, Qt::WindowFlags fl)
  : QDialog(parent_, fl) {
  ui_.setupUi(this);
  setWindowTitle(parent_->windowTitle());
  strUserName = username;
  strPassword = password;
  ui_.lePassword->setEchoMode(QLineEdit::Password);
  ui_.leUserName->setText(*username);
  ui_.lePassword->setText(*password);
  if (!username->isEmpty()) {
    ui_.leUserName->setDisabled(true);
    ui_.lePassword->setFocus();
  } else {
    ui_.leUserName->setDisabled(false);
    ui_.leUserName->setFocus();
  }

  FQ_VERIFY(connect(ui_.bOK, SIGNAL(clicked()), this, SLOT(accept())));
  FQ_VERIFY(connect(ui_.bCancel, SIGNAL(clicked()), this, SLOT(reject())));
  FQ_VERIFY(connect(ui_.leUserName, SIGNAL(returnPressed()), this, SLOT(moveFocus())));
  FQ_VERIFY(connect(ui_.lePassword, SIGNAL(returnPressed()), this, SLOT(accept())));
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SSHLoginDialog::~SSHLoginDialog() {
	// no need to delete child widgets, Qt does it all for us
}

void SSHLoginDialog::accept() {
	*strUserName = ui_.leUserName->text();
	*strPassword = ui_.lePassword->text();
	QDialog::accept();
}

void SSHLoginDialog::moveFocus()
{
  ui_.lePassword->setFocus();
}

}  // namespace FQTerm

#include "sshlogindialog.moc"
