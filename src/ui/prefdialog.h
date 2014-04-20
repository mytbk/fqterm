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

#ifndef FQTERM_PREF_DIALOG_H
#define FQTERM_PREF_DIALOG_H

#include "ui_prefdialog.h"
#include "fqterm_filedialog.h"

class QCloseEvnt;

namespace FQTerm {

class FQTermConfig;

class prefDialog: public QDialog {
  Q_OBJECT;
 public:
  prefDialog(FQTermConfig *, QWidget *parent_ = 0, Qt::WindowFlags fl = 0);
  ~prefDialog();

 protected slots:
  void onOK();
  void onCancel();
  void onSound();
  void onHttp();
  // 	void onBeep(int);
  void onBrowse();
  void onImage();
  void onPool();
  void onStyleSheet();
  void onEditor();
 protected:
  void closeEvent(QCloseEvent*);
  void connectSlots();
  void loadSetting();
  void saveSetting();

 private:
  Ui::prefDialog ui_;
  QButtonGroup soundButtonGroup_;
  QButtonGroup verticalSettingButtonGroup_;
  FQTermConfig * config_;
  FQTermFileDialog *fileDialog_;
};

}  // namespace FQTerm

#endif  // FQTERM_PREF_DIALOG_H
