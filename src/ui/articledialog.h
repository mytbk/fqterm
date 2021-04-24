// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_ARTICLEDIALOG_H
#define FQTERM_ARTICLEDIALOG_H

#include "ui_articledialog.h"

namespace FQTerm {

class FQTermConfig;
class FQTermFileDialog;

class articleDialog: public QDialog {
  Q_OBJECT; 
public:
  articleDialog(FQTermConfig *, QWidget *parent_ = 0, Qt::WindowFlags fl = 0);
  ~articleDialog();

  QString articleText_;
  Ui::articleDialog ui_;
protected:
  void connectSlots();
protected slots:
  void onClose();
  void onSave();
  void onSelect();
  void onCopy();
private:
  FQTermConfig * config_;
};

} // name space FQTerm

#endif  // FQTERM_ARTICLEDIALOG_H
