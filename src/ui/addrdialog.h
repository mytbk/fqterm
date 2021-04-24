// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_ADDR_DIALOG_H
#define FQTERM_ADDR_DIALOG_H

#include <QButtonGroup>

#include "fqterm_param.h"
#include "ui_addrdialog.h"

namespace FQTerm {

class FQTermConfig;

class addrDialog: public QDialog {
  Q_OBJECT;
 public:
   enum BUTTONS{SAVE = 0x01, APPLY = 0x02};
  addrDialog(QWidget *parent_ = 0, const FQTermParam& param = FQTermParam(),
             int buttons = SAVE | APPLY, Qt::WindowFlags fl = 0);
  ~addrDialog();
  void setParam(const FQTermParam& param) {
    param_ = param;
  }
  FQTermParam param() {
    return param_;
  }

  enum Tabs{General, Display, Terminal, Keyboard, Proxy, Misc, Mouse};
  void setCurrentTabIndex(Tabs tab) {
    ui_.tabWidget->setCurrentIndex(tab);
  }

  static const int ports[3]; // telnet, ssh1, ssh2

 protected slots:
  void onOK();
  void onSave();
  void onCancel();
  void onFgcolor();
  void onBgcolor();
  void onProtocol(int);
  void onChooseScript();
  void onMenuColor();
  void onFont();

 protected:
  void connector();
  bool isChanged();
  void previewFont();
  void previewMenu();
  void setUIFromParam();
  void setParamFromUI();

  QButtonGroup menuButtonGroup_;

 private:
  
  FQTermParam param_;
  Ui::addrDialog ui_;

  void updateSchemaList(const QString& currentFile);
};




}  // namespace FQTerm

#endif  // FQTERM_ADDR_DIALOG_H
