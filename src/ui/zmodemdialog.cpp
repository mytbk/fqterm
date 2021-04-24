// SPDX-License-Identifier: GPL-2.0-or-later

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
  QMessageBox mb("FQTerm", "We don't support cancel operation yet. "
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
