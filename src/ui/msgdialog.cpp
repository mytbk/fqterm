// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdio.h>

#include "msgdialog.h"

msgDialog::msgDialog(QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl) {
  ui_.setupUi(this);
  // signals and slots connections
  //QVERIFY(connect( okButton, SIGNAL( clicked() ), this, SLOT( accept() ) ));
}

/*  
 *  Destroys the object and frees any allocated resources
 */
msgDialog::~msgDialog() {
  // no need to delete child widgets, Qt does it all for us
}

void msgDialog::setMessageText(const QString& message) {
  ui_.msgBrowser->setPlainText(message);
}

#include "msgdialog.moc"
