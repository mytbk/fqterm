// SPDX-License-Identifier: GPL-2.0-or-later

#include "iplookup.h"
#include "fqterm_ip_location.h"
#include "fqterm.h"

#if QT_VERSION >= 0x050000
#define FQ_UTF8 0
#else
#define FQ_UTF8 QCoreApplication::UnicodeUTF8
#endif

namespace FQTerm {



IPLookupDialog::~IPLookupDialog(){

}

IPLookupDialog::IPLookupDialog( QWidget *parent_ /*= 0*/, Qt::WindowFlags fl /*= 0*/ )  : QDialog(parent_, fl) {
  ui_.setupUi(this);
  FQ_VERIFY(connect(ui_.lookupPushButton, SIGNAL(clicked()), this, SLOT(onLookup())));
  FQ_VERIFY(connect(ui_.finishedPushButton, SIGNAL(clicked()), this, SLOT(onFinished())));
}

void IPLookupDialog::onLookup() {
  QString country, city;
  QString ip = ui_.ipLineEdit->text();
  FQTermIPLocation *ipLocation = FQTermIPLocation::getInstance();
  if (ipLocation == NULL) {
    ui_.addressLineEdit->setText(
        QApplication::translate("IPLookupDialog", "IP database file does NOT exist", 0, FQ_UTF8));
  } else if (!ipLocation->getLocation(ip, country, city)) {
    ui_.addressLineEdit->setText(
        QApplication::translate("IPLookupDialog", "Invalid IP", 0, FQ_UTF8));
  } else {
    ui_.addressLineEdit->setText(country + " " + city);
  }
}

void IPLookupDialog::onFinished() {
  done(0);
}
} //namespace FQTerm

#include "iplookup.moc"
