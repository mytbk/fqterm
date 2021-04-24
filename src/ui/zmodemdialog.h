// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_ZMODEM_DIALOG_H
#define FQTERM_ZMODEM_DIALOG_H

#include "ui_zmodemdialog.h"

namespace FQTerm {

class zmodemDialog: public QDialog {
  Q_OBJECT;
 public:
  zmodemDialog(QWidget *parent_ = 0, Qt::WindowFlags fl = 0);
  ~zmodemDialog();

  void setFileInfo(const QString &, int);
  void addErrorLog(const QString &);
  void clearErrorLog();
  void setProgress(int);

 public slots:
  void slotCancel();

 signals:
  void canceled();

 private:
  Ui::zmodemDialog ui_;
};

}  // namespace FQTerm

#endif  // FQTERM_ZMODEM_DIALOG_H
