// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_KEY_DIALOG_H
#define FQTERM_KEY_DIALOG_H

#include "ui_keydialog.h"
#include <QButtonGroup>

namespace FQTerm {

class FQTermConfig;

class keyDialog: public QDialog {
  Q_OBJECT;
 public:
  keyDialog(FQTermConfig *, QWidget *parent_ = 0, Qt::WindowFlags fl = 0);
  ~keyDialog();

 protected:
  void connectSlots();
  void loadKey(int);
  void loadName();

 protected slots:
  void onNamechange(int);
  void onClose();
  void onAdd();
  void onDelete();
  void onUpdate();
  void onChooseScript();
  void onProgram();
  void onUp();
  void onDown();
  void onLeft();
  void onRight();
  void onEnter();
  // 	void onSelect(int);
 private:
  Ui::keyDialog ui_;
  QButtonGroup keyButtonGroup_;
  FQTermConfig * config_;
};

}  // namespace FQTerm

#endif  // FQTERM_KEY_DIALOG_H
