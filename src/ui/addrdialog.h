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
