// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SOUND_CONF_H
#define FQTERM_SOUND_CONF_H

#include <QButtonGroup>
#include "ui_soundconf.h"
#include "fqterm_filedialog.h"

namespace FQTerm {

class FQTermSound;
class FQTermConfig;

class soundConf: public QDialog {
  Q_OBJECT;
 public:
  soundConf(FQTermConfig *, QWidget *parent_ = 0, Qt::WindowFlags fl = 0);
  ~soundConf();
  void loadSetting();
  void saveSetting();

 public slots:
  void onSelectFile();
  void onSelectProg();
  void onPlayMethod(int id);
  void onTestPlay();
 protected slots:
  void accept();

 private:
  FQTermSound *sound_;
  Ui::soundConf ui_;
  QButtonGroup buttonGroup_;
  FQTermConfig * config_;
};

}  // namespace FQTerm

#endif  // FQTERM_SOUND_CONF_H
