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

#include <QMessageBox>
#include <QFileDialog>

#include "fqterm.h"
#include "fqterm_config.h"
#include "fqterm_path.h"
#include "fqterm_sound.h"

#include "soundconf.h"

namespace FQTerm {
/*
 *  Constructs a fSoundConf which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */

soundConf::soundConf(FQTermConfig * config, QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl),
    buttonGroup_(this),
    config_(config){
  ui_.setupUi(this);
  buttonGroup_.addButton(ui_.radioButton1, 0);
  buttonGroup_.addButton(ui_.radioButton2, 1);
  sound_ = NULL;
  loadSetting();
  FQ_VERIFY(connect(ui_.bfSelect, SIGNAL(clicked()),
    this, SLOT(onSelectFile())));
  FQ_VERIFY(connect(ui_.bpSelect, SIGNAL(clicked()),
    this, SLOT(onSelectProg())));
  FQ_VERIFY(connect(ui_.bpTest, SIGNAL(clicked()),
    this, SLOT(onTestPlay())));
  FQ_VERIFY(connect(ui_.bOK, SIGNAL(clicked()),
    this, SLOT(accept())));
  FQ_VERIFY(connect(ui_.bCancel, SIGNAL(clicked()),
    this, SLOT(reject())));
}

/*
 *  Destroys the object and frees any allocated resources
 */
soundConf::~soundConf() {
  // no need to delete child widgets, Qt does it all for us
}

/*
 * public slot
 */
void soundConf::onSelectFile() {
  FQTermFileDialog fileDialog(config_);
  QString soundfile = fileDialog.getOpenName("Choose a WAVE file", "WAVE Audio Files (*.wav *.WAV)");
  if (!soundfile.isEmpty()) {
    ui_.leFile->setText(soundfile);
  }
}

/*
 * public slot
 */
void soundConf::onSelectProg() {
  FQTermFileDialog fileDialog(config_);
  QString progfile = fileDialog.getOpenName("Choose a program", "");
  if (!progfile.isEmpty()) {
    ui_.leProg->setText(progfile);
  }
}

/*
 * public slot
 */
void soundConf::onPlayMethod(int id) {
FQ_TRACE("sconf", 0) << id << ": " << buttonGroup_.checkedId();
  ui_.bpSelect->setEnabled(id == 1 || buttonGroup_.checkedId() == 1);
}

void soundConf::onTestPlay() {
  if (ui_.leFile->text().isEmpty()) {
    QMessageBox::critical(this, tr("No sound file"),
      tr("You have to select a file to test the sound"), tr("&Ok"));

    return;
  }

  sound_ = NULL;

  switch (buttonGroup_.checkedId()) {
    case 0:
      sound_ = new FQTermSystemSound(ui_.leFile->text());
      break;
    case 1:
      if (ui_.leProg->text().isEmpty()) {
        QMessageBox::critical(this, tr("No player"),
          tr("You have to specify an external player"), tr("&Ok"));

        break;
      }

      sound_ = new FQTermExternalSound(ui_.leProg->text(), ui_.leFile->text());
      break;
  }

  if (sound_) {
    sound_->start();
  }
}

void soundConf::loadSetting() {

  QString strTmp;

  strTmp = config_->getItemValue("preference", "wavefile");
  if (!strTmp.isEmpty()) {
    ui_.leFile->setText(strTmp);
  }

  strTmp = config_->getItemValue("preference", "playmethod");

  int valTmp = !strTmp.isEmpty()? strTmp.toInt(): -1;

  if (valTmp >= 0 && valTmp <= 1) {
    buttonGroup_.button(valTmp)->setChecked(true);
    if (valTmp == 1) {
      strTmp = config_->getItemValue("preference", "externalplayer");
      if (!strTmp.isEmpty()) {
        ui_.leProg->setText(strTmp);
      }
    }
  }
}

void soundConf::saveSetting() {

  QString strTmp;

  config_->setItemValue("preference", "beep", "2");

  config_->setItemValue("preference", "wavefile", ui_.leFile->text());

  strTmp.setNum(buttonGroup_.checkedId());
  config_->setItemValue("preference", "playmethod", strTmp);

  if (strTmp == "1") {
    config_->setItemValue("preference", "externalplayer", ui_.leProg->text());
  }

  config_->save(getPath(USER_CONFIG) + "fqterm.cfg");
}

void soundConf::accept() {
  saveSetting();
  QDialog::accept();
}

}  // namespace FQTerm

#include "soundconf.moc"
