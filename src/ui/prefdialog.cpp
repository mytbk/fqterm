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

#include <QCloseEvent>
#include <QComboBox>
#include <QFileDialog>

#include "fqterm_config.h"
#include "fqterm_path.h"
#include "fqterm_trace.h"

#include "prefdialog.h"
#include "soundconf.h"

namespace FQTerm {

/*
 *  Constructs a prefDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
prefDialog::prefDialog(FQTermConfig * config, QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl),
      soundButtonGroup_(this),
      verticalSettingButtonGroup_(this),
      config_(config){
  ui_.setupUi(this);
  fileDialog_ = new FQTermFileDialog(config_);
  soundButtonGroup_.addButton(ui_.noneRadioButton, 0);
  soundButtonGroup_.addButton(ui_.beepRadioButton, 1);
  soundButtonGroup_.addButton(ui_.fileRadioButton, 2);
  verticalSettingButtonGroup_.addButton(ui_.topRadioButton, 0);
  verticalSettingButtonGroup_.addButton(ui_.middleRadioButton, 1);
  verticalSettingButtonGroup_.addButton(ui_.bottomRadioButton, 2);

  connectSlots();

  loadSetting();

}

/*
 *  Destroys the object and frees any allocated resources
 */
prefDialog::~prefDialog() {
  // no need to delete child widgets, Qt does it all for us
  delete fileDialog_;
}

void prefDialog::connectSlots() {
  FQ_VERIFY(connect(ui_.okPushButton, SIGNAL(clicked()), this, SLOT(onOK())));
  FQ_VERIFY(connect(ui_.cancelPushButton, SIGNAL(clicked()), this, SLOT(onCancel())));
  FQ_VERIFY(connect(ui_.selectsoundPushButton, SIGNAL(clicked()), this, SLOT(onSound())));
  FQ_VERIFY(connect(ui_.choosehttpPushButton, SIGNAL(clicked()), this, SLOT(onHttp())));
  //FQ_VERIFY(connect(ButtonGroup1 , SIGNAL(clicked(int)), this, SLOT(onBeep(int)) ));
  FQ_VERIFY(connect(ui_.browsePushButton, SIGNAL(clicked()), this, SLOT(onBrowse())));
  FQ_VERIFY(connect(ui_.imagePushButton, SIGNAL(clicked()), this, SLOT(onImage())));
  FQ_VERIFY(connect(ui_.poolPushButton, SIGNAL(clicked()), this, SLOT(onPool())));
  FQ_VERIFY(connect(ui_.styleSheetPushButton, SIGNAL(clicked()), this, SLOT(onStyleSheet())));
  FQ_VERIFY(connect(ui_.editorPushButton, SIGNAL(clicked()), this, SLOT(onEditor())));
}

void prefDialog::loadSetting() {

  QString strTmp;

  strTmp = config_->getItemValue("preference", "displayoffset");
  ui_.displayOffsetSlider->setSliderPosition(strTmp.toInt());

  strTmp = config_->getItemValue("preference", "xim");
  ui_.ximComboBox->setCurrentIndex(strTmp.toInt());

  strTmp = config_->getItemValue("preference", "wordwrap");
  ui_.wordLineEdit3->setText(strTmp);

  strTmp = config_->getItemValue("preference", "smartww");
  ui_.smartCheckBox->setChecked(strTmp != "0");

  strTmp = config_->getItemValue("preference", "wheel");
  ui_.wheelCheckBox->setChecked(strTmp != "0");

  strTmp = config_->getItemValue("preference", "url");
  ui_.urlCheckBox->setChecked(strTmp != "0");

  strTmp = config_->getItemValue("preference", "logmsg");
  ui_.saveCheckBox->setChecked(strTmp != "0");

  strTmp = config_->getItemValue("preference", "blinktab");
  ui_.blinkCheckBox->setChecked(strTmp != "0");

  strTmp = config_->getItemValue("preference", "warn");
  ui_.warnCheckBox->setChecked(strTmp != "0");

  strTmp = config_->getItemValue("preference", "beep");
  qobject_cast<QRadioButton *>(soundButtonGroup_.button(strTmp.toInt()))->setChecked(true);

  strTmp = config_->getItemValue("preference", "vsetting");
  qobject_cast<QRadioButton *>(verticalSettingButtonGroup_.button(strTmp.toInt()))->setChecked(true);

  strTmp = config_->getItemValue("preference", "enq");
  ui_.enqCheckBox->setChecked(strTmp != "0");

  //ButtonGroup1->find(strTmp.toInt()))->setChecked(true);

  // 	if(strTmp.toInt()!=2)
  // 	{
  // 		wavefileLineEdit->setEnabled(false);
  // 		selectsoundPushButton->setEnabled(false);
  // 	}

  strTmp = config_->getItemValue("preference", "wavefile");
  ui_.wavefileLineEdit->setText(strTmp);

  strTmp = config_->getItemValue("preference", "antialias");
  ui_.aacheckBox->setChecked(strTmp != "0");


  strTmp = config_->getItemValue("preference", "tray");
  ui_.trayCheckBox->setChecked(strTmp != "0");

  strTmp = config_->getItemValue("preference", "clearpool");
  ui_.clearCheckBox->setChecked(strTmp == "1");

  strTmp = config_->getItemValue("preference", "pool");
  if (strTmp.isEmpty()) {
    strTmp = getPath(USER_CONFIG) + "pool/";
  }
  ui_.poolLineEdit->setText(strTmp);

  strTmp = config_->getItemValue("preference", "http");
  ui_.httpLineEdit->setText(strTmp);

  strTmp = config_->getItemValue("preference", "zmodem");
  if (strTmp.isEmpty()) {
    strTmp = getPath(USER_CONFIG) + "zmodem/";
  }
  ui_.zmodemLineEdit->setText(strTmp);

  strTmp = config_->getItemValue("preference", "image");
  ui_.imageLineEdit->setText(strTmp);

  strTmp = config_->getItemValue("preference", "qssfile");
  ui_.styleSheetLineEdit->setText(strTmp);

  strTmp = config_->getItemValue("preference", "editor");
  ui_.editorLineEdit->setText(strTmp);

  strTmp = config_->getItemValue("preference", "editorarg");
  ui_.editorArgLineEdit->setText(strTmp);

  strTmp = config_->getItemValue("preference", "asciienhance");
  ui_.asciiEnhanceCheckBox->setChecked(strTmp == "1");
}

void prefDialog::saveSetting() {

  QString strTmp;

  strTmp.setNum(ui_.displayOffsetSlider->sliderPosition());
  config_->setItemValue("preference", "displayoffset", strTmp);

  strTmp.setNum(ui_.ximComboBox->currentIndex());
  config_->setItemValue("preference", "xim", strTmp);


  config_->setItemValue("preference", "wordwrap", ui_.wordLineEdit3->text());

  strTmp.setNum(ui_.smartCheckBox->isChecked() ? 1 : 0);
  config_->setItemValue("preference", "smartww", strTmp);

  strTmp.setNum(ui_.wheelCheckBox->isChecked() ? 1 : 0);
  config_->setItemValue("preference", "wheel", strTmp);

  strTmp.setNum(ui_.urlCheckBox->isChecked() ? 1 : 0);
  config_->setItemValue("preference", "url", strTmp);

  strTmp.setNum(ui_.saveCheckBox->isChecked() ? 1 : 0);
  config_->setItemValue("preference", "logmsg", strTmp);

  strTmp.setNum(ui_.blinkCheckBox->isChecked() ? 1 : 0);
  config_->setItemValue("preference", "blinktab", strTmp);

  strTmp.setNum(ui_.warnCheckBox->isChecked() ? 1 : 0);
  config_->setItemValue("preference", "warn", strTmp);

  strTmp.setNum(ui_.aacheckBox->isChecked() ? 1 : 0);
  config_->setItemValue("preference", "antialias", strTmp);

  strTmp.setNum(ui_.trayCheckBox->isChecked() ? 1 : 0);
  config_->setItemValue("preference", "tray", strTmp);

  strTmp.setNum(ui_.enqCheckBox->isChecked() ? 1 : 0);
  config_->setItemValue("preference", "enq", strTmp);

  strTmp.setNum(soundButtonGroup_.checkedId());
  config_->setItemValue("preference", "beep", strTmp);

  strTmp.setNum(verticalSettingButtonGroup_.checkedId());
  config_->setItemValue("preference", "vsetting", strTmp);

  if (strTmp == "2") {
    config_->setItemValue("preference", "wavefile", ui_.wavefileLineEdit->text());
  }

  strTmp.setNum(ui_.clearCheckBox->isChecked() ? 1 : 0);
  config_->setItemValue("preference", "clearpool", strTmp);

  strTmp.setNum(ui_.asciiEnhanceCheckBox->isChecked() ? 1 : 0);
  config_->setItemValue("preference", "asciienhance", strTmp);

  strTmp = ui_.poolLineEdit->text();
  if (strTmp.isEmpty()) {
    strTmp = getPath(USER_CONFIG) + "pool/";
  }
  config_->setItemValue("preference", "pool", strTmp);

  strTmp = ui_.zmodemLineEdit->text();
  if (strTmp.isEmpty()) {
    strTmp = getPath(USER_CONFIG) + "zmodem/";
  }
  config_->setItemValue("preference", "zmodem", strTmp);
  config_->setItemValue("preference", "http", ui_.httpLineEdit->text());
  config_->setItemValue("preference", "image", ui_.imageLineEdit->text());

  config_->setItemValue("preference", "qssfile", ui_.styleSheetLineEdit->text().trimmed());

  config_->setItemValue("preference", "editor", ui_.editorLineEdit->text());
  config_->setItemValue("preference", "editorarg", ui_.editorArgLineEdit->text());
  
  config_->save(getPath(USER_CONFIG) + "fqterm.cfg");
}

void prefDialog::closeEvent(QCloseEvent*) {
  reject();
}

void prefDialog::onOK() {
  saveSetting();
  done(1);
}

void prefDialog::onCancel() {
  done(0);
}

void prefDialog::onSound() {
  soundConf soundconf(config_, this);
  if (soundconf.exec() == 1) {
    loadSetting();
  }

}

void prefDialog::onHttp() {
  QString http = fileDialog_->getOpenName("Choose a WWW browser", "*");
  if (!http.isEmpty()) {
    ui_.httpLineEdit->setText(http);
  }

}

// void prefDialog::onBeep( int id )
// {
// 	if(id==2)
// 	{
// 		ui.wavefileLineEdit->setEnabled(true);
// 		ui.selectsoundPushButton->setEnabled(true);
// 	}
// 	else if(id==0 || id==1 )
// 	{
// 		ui.wavefileLineEdit->setEnabled(false);
// 		ui.selectsoundPushButton->setEnabled(false);
// 	}
// }

void prefDialog::onBrowse() {
  QString dir = fileDialog_->getExistingDirectory("Choose a directory", ui_.zmodemLineEdit->text());
  if (!dir.isEmpty()) {
    ui_.zmodemLineEdit->setText(dir);
  }
}

void prefDialog::onImage() {
  QString image = fileDialog_->getOpenName("Choose an Image Viewer", "*");
  if (!image.isEmpty()) {
    ui_.imageLineEdit->setText(image);
  }
}

void prefDialog::onPool() {
  QString pool = fileDialog_->getExistingDirectory("Choose a directory", ui_.poolLineEdit->text());
  if (!pool.isEmpty()) {
    ui_.poolLineEdit->setText(pool);
  }
}

void prefDialog::onStyleSheet()
{
  QString qssFile = fileDialog_->getOpenName("Choose a QSS File", "Qt Style Sheets (*.qss *.QSS)");
  if (!qssFile.isEmpty()) {
    ui_.styleSheetLineEdit->setText(qssFile);
  }
}

void prefDialog::onEditor() {
  QString editor = fileDialog_->getOpenName("Choose a directory", ui_.editorLineEdit->text());
  if (!editor.isEmpty()) {
    ui_.editorLineEdit->setText(editor);
  }
}
}  // namespace FQTerm

#include "prefdialog.moc"
