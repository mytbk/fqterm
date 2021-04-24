// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_PREF_DIALOG_H
#define FQTERM_PREF_DIALOG_H

#include <QButtonGroup>
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
