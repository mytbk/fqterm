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

#include <QMessageBox>

#include "fqterm_trace.h"
#include "zmodemdialog.h"

namespace FQTerm {

zmodemDialog::zmodemDialog(QWidget *parent_, Qt::WindowFlags fl)
    : QDialog(parent_, fl) {
  ui_.setupUi(this);
  FQ_VERIFY(connect(ui_.buttonCancel, SIGNAL(clicked()), this, SLOT(slotCancel())));
}

zmodemDialog::~zmodemDialog(){}

void zmodemDialog::setProgress(int offset) {
  ui_.pbProgress->setValue(offset);
  QString strTmp;
  strTmp = QString("%1 out of %2 bytes").arg(offset).arg(ui_.pbProgress->maximum()
                                                         );
  ui_.labelStatus->setText(strTmp);
}

void zmodemDialog::setFileInfo(const QString &name, int size) {
  ui_.labelFileName->setText(name);
  ui_.pbProgress->setMaximum(size);
}

void zmodemDialog::addErrorLog(const QString &err) {
  // FIXME:display error message;
  //browserError->append(err);
}

void zmodemDialog::clearErrorLog() {
  //browserError->clear();
}

void zmodemDialog::slotCancel() {
  QMessageBox mb("FQTerm", "We dont support cancel operation yet. "
                 "But you can try, it will crash when downloading.\n"
                 "Do you want to continue?",
                 QMessageBox::Warning, QMessageBox::Yes,
                 QMessageBox::No | QMessageBox::Escape | QMessageBox::Default,
                 0, this);
  if (mb.exec() == QMessageBox::Yes) {
    emit canceled();
    hide();
  }
}

}  // namespace FQTerm

#include "zmodemdialog.moc"
