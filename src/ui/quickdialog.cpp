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

#include <stdio.h>

#include <QCloseEvent>
#include <QComboBox>
#include <QPixmap>
#include <QImage>
#include <QPixmap>
#include <QMessageBox>

#include "fqterm_trace.h"
#include "fqterm_path.h"
#include "fqterm_config.h"
#include "fqterm_param.h"
#include "addrdialog.h"
#include "quickdialog.h"

namespace FQTerm {

//extern QString fileCfg;
//extern QString addrCfg;

extern void saveAddress(FQTermConfig *, int, const FQTermParam &);

  const int quickDialog::ports[] = {23, 22, 22, 22};

quickDialog::quickDialog(FQTermConfig * config, QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl),
      config_(config) {
  ui_.setupUi(this);

  ui_.addPushButton->setIcon(QPixmap(getPath(RESOURCE) + "pic/address_book.png"));

  ui_.addPushButton->setToolTip(tr("Add To AddressBook"));

  ui_.connectPushButton->setDefault(true);

  connectSlots();

  loadHistory();

//  adjustSize();
//  setFixedSize(size());
  
}

quickDialog::~quickDialog() {
}

void quickDialog::closeEvent(QCloseEvent*) {
  config_->save(getPath(USER_CONFIG) + "fqterm.cfg");
  reject();
}

void quickDialog::loadHistory() {
  QString defaultIndex = config_->getItemValue("global", "quickdefaultindex");
  if (defaultIndex.isEmpty()) {
    config_->setItemValue("global", "quick default index", "0");
    defaultIndex = "0";
  }
  QString strTmp = config_->getItemValue("quick list", "num");
  QString strSection;
  for (int i = 0; i < strTmp.toInt(); i++) {
    strSection = QString("quick %1").arg(i);
    ui_.historyComboBox->addItem(config_->getItemValue(strSection.toLatin1(), "addr"));
  }

  int index = defaultIndex.toInt();
  if (strTmp.toInt() > index) {
    ui_.historyComboBox->setCurrentIndex(index);
    listChanged(index);
  }
}

void quickDialog::connectSlots() {
  FQ_VERIFY(connect(ui_.historyComboBox, SIGNAL(activated(int)), this, SLOT(listChanged(int))));
  FQ_VERIFY(connect(ui_.protocolComboBox, SIGNAL(activated(int)), this, SLOT(onSelectProtocol(int))));
  FQ_VERIFY(connect(ui_.addPushButton, SIGNAL(clicked()), this, SLOT(onAdd())));
  FQ_VERIFY(connect(ui_.deletePushButton, SIGNAL(clicked()), this, SLOT(onDelete())));
  FQ_VERIFY(connect(ui_.advPushButton, SIGNAL(clicked()), this, SLOT(onAdvance())));
  FQ_VERIFY(connect(ui_.connectPushButton, SIGNAL(clicked()), this, SLOT(onConnect())));
  FQ_VERIFY(connect(ui_.closePushButton, SIGNAL(clicked()), this, SLOT(onClose())));
  FQ_VERIFY(connect(ui_.portCheck, SIGNAL(toggled(bool)), this, SLOT(portCheckToggled(bool))));
}

void quickDialog::portCheckToggled(bool checked)
{
  if (checked){
    ui_.portEdit->setEnabled(true);
  }else{
    ui_.portEdit->setEnabled(false);
    int index = ui_.protocolComboBox->currentIndex();
    ui_.portEdit->setText(QString("%1").arg(ports[index]));
  }
}

void quickDialog::listChanged(int index) {
  QString strTmp = config_->getItemValue("quick list", "num");
  if (strTmp == "0") {
    return ;
  }
  loadParam(index);
  setUIFromParam();
}

void quickDialog::onAdd() {
  setParamFromUI();

  FQTermConfig *pAddrConf = new FQTermConfig(getPath(USER_CONFIG) + "address.cfg");
  QString strTmp;
  strTmp = pAddrConf->getItemValue("bbs list", "num");
  int num = strTmp.toInt();
  strTmp.setNum(num + 1);
  pAddrConf->setItemValue("bbs list", "num", strTmp);
  saveAddress(pAddrConf, num, param_);
  pAddrConf->save(getPath(USER_CONFIG) + "address.cfg");
}

void quickDialog::onDelete() {
  int index = ui_.historyComboBox->currentIndex();

  QString strTmp = config_->getItemValue("quick list", "num");
  int num = strTmp.toInt();

  if (num != 0 && index != -1) {
    QString strSection = QString("quick %1").arg(index);
    if (!config_->deleteSection(strSection.toLatin1())) {
      return ;
    }
    ui_.historyComboBox->removeItem(index);

    for (int i = index + 1; i < num; i++) {
      strTmp = QString("quick %1").arg(i);
      strSection = QString("quick %1").arg(i - 1);
      config_->renameSection(strTmp.toLatin1(), strSection.toLatin1());
    }

    strTmp = config_->getItemValue("quick list", "num");
    strTmp.setNum(qMax(0, strTmp.toInt() - 1));
    config_->setItemValue("quick list", "num", strTmp.toLatin1());

    // update
    if (num > 1) {
      ui_.historyComboBox->setCurrentIndex(qMin(index, num - 2));
      listChanged(qMin(index, num - 2));
    }
  }
}

void quickDialog::onAdvance() {
  setParamFromUI();

  addrDialog set(this, param_, addrDialog::APPLY);

  if (set.exec() == 1) {
    param_ = set.param();
    setUIFromParam();
  }
}

void quickDialog::onConnect() {
  if (ui_.historyComboBox->currentText().isEmpty() || ui_.portEdit->text().isEmpty()) {
    QMessageBox mb("FQTerm", "address or port cant be blank", QMessageBox::Warning,
                   QMessageBox::Ok | QMessageBox::Default, 0, 0);
    mb.exec();
    return ;
  }

  setParamFromUI();
  FQTermParam newParam = param_;
  loadParam(ui_.historyComboBox->currentIndex());

  QString strTmp = config_->getItemValue("quick list", "num");
  if (strTmp.isEmpty() || strTmp == "-1") {
    strTmp = "0";
  }
  int num = strTmp.toInt();
  if (ui_.historyComboBox->currentIndex() == -1 ||
    newParam.hostAddress_ != param_.hostAddress_ || 
    newParam.protocolType_ != param_.protocolType_ ||
    newParam.port_ != param_.port_) {
    //changed. add new info
    param_ = newParam;
    saveParam(num);
    ui_.historyComboBox->addItem(param_.hostAddress_);
    ui_.historyComboBox->setCurrentIndex(num);
    config_->setItemValue("quick list", "num", QString("%1").arg(num + 1));
  } else {
    param_ = newParam;
  }

  config_->setItemValue("global", "quickdefaultindex",
  QString("%1").arg(ui_.historyComboBox->currentIndex()));
  done(1);
}

void quickDialog::onClose() {
  done(0);
}

void quickDialog::onSelectProtocol(int index)
{
  ui_.portCheck->setChecked(false);
  ui_.portEdit->setText(QString("%1").arg(ports[index]));
}

void quickDialog::setUIFromParam()
{
  ui_.historyComboBox->setItemText(ui_.historyComboBox->currentIndex(), param_.hostAddress_);
  ui_.protocolComboBox->setCurrentIndex(param_.protocolType_);
  ui_.portEdit->setText(QString("%1").arg(param_.port_));
  if (param_.port_ != ports[param_.protocolType_]) {
    ui_.portCheck->setChecked(true);
  }
  else {
    ui_.portCheck->setChecked(false);
  }
}

void quickDialog::setParamFromUI()
{
  param_.name_ = ui_.historyComboBox->currentText();
  param_.hostAddress_ = ui_.historyComboBox->currentText();
  param_.protocolType_ = ui_.protocolComboBox->currentIndex();
  bool ok;
  param_.port_ = ui_.portEdit->text().toInt(&ok);
  if (!ok) {
    param_.port_ = ports[param_.protocolType_];
  }
}

void quickDialog::loadParam(int index)
{
  param_ = FQTermParam();
  QString strSection = QString("quick %1").arg(index);
  param_.name_ = config_->getItemValue(strSection, "addr");
  param_.hostAddress_ = param_.name_;
  param_.protocolType_ = config_->getItemValue(strSection, "protocol").toInt();
  param_.port_ = config_->getItemValue(strSection, "port").toInt();
}

void quickDialog::saveParam(int index)
{
  QString strSection = QString("quick %1").arg(index);
  config_->setItemValue(strSection, "addr", param_.hostAddress_);
  config_->setItemValue(strSection, "protocol", QString("%1").arg(param_.protocolType_));
  config_->setItemValue(strSection, "port", QString("%1").arg(param_.port_));
}
}  // namespace FQTerm

#include "quickdialog.moc"
