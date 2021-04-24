// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_QUICK_DIALOG_H
#define FQTERM_QUICK_DIALOG_H

#include "fqterm_param.h"
#include "ui_quickdialog.h"
//Added by qt3to4:
//#include <QCloseEvent>

class QCloseEvent;
class QResizeEvent;

namespace FQTerm {

class FQTermConfig;

class quickDialog: public QDialog {
  Q_OBJECT;
 public:
  quickDialog(FQTermConfig *, QWidget *parent_ = 0, Qt::WindowFlags fl = 0);
  ~quickDialog();

  FQTermParam param_;

  static const int ports[];

 protected slots:
  void listChanged(int);  //
  void onAdd();
  void onDelete();
  void onAdvance();
  void onConnect(); //
  void onClose();
  void onSelectProtocol(int index);
  void portCheckToggled(bool);

 private:

  void closeEvent(QCloseEvent*);  //
  void connectSlots();
  void loadHistory(); //
  void setUIFromParam();
  void setParamFromUI();
  void loadParam(int index);
  void saveParam(int index);

  FQTermConfig *config_;

 private:
  Ui::quickDialog ui_;
};

}  // namespace FQTerm

#endif // FQTERM_QUICK_DIALOG_H
