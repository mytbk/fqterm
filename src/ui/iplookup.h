// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef __FQTERM_IPLOOKUP__
#define __FQTERM_IPLOOKUP__

#include <QDialog>
#include "ui_iplookup.h"
namespace FQTerm {

class IPLookupDialog : public QDialog {
  Q_OBJECT;
public:
  IPLookupDialog(QWidget *parent_ = 0, Qt::WindowFlags fl = 0);
  ~IPLookupDialog();

protected slots:
  void onLookup();
  void onFinished();
private:
  Ui::IPLookupDialog ui_;
};

} //namespace FQTerm

#endif //__FQTERM_IPLOOKUP__