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

#include <QTextCharFormat>
#include <QDialog>
#include <QIntValidator>
#include <QPainter>
#include <QResizeEvent>
#include <QPalette>
#include <QListWidgetItem>
#include <QMessageBox>

#include "fqterm_config.h"
#include "fqterm_param.h"
#include "fqterm_param.h"
#include "fqterm_path.h"
#include "fqterm_trace.h"

#include "sitemanager.h"

namespace FQTerm{

int siteDialog::ports[] = {23, 22, 22, 0};

siteDialog::siteDialog(QWidget *parent_ /*= 0*/, Qt::WindowFlags fl /*= 0*/) 
  : QDialog(parent_, fl) {
  ui_.setupUi(this);

  config_ = new FQTermConfig(getPath(USER_CONFIG) + "address.cfg");
  QStringList name_list;
  loadNameList(config_, name_list);
  ui_.siteList->addItems(name_list);

  if (ui_.siteList->count() != 0) {
    setCurrentSite(0);
    loadCurrentParam();
    setUIFromParam();
  }
  else {
    ui_.editGroup->setDisabled(true);
  }
  connector();
}

siteDialog::~siteDialog() {
  config_->save(getPath(USER_CONFIG) + "address.cfg");
  delete config_;
}

void siteDialog::onSelectProtocol(int index) {
  ui_.portCheck->setChecked(false);
  ui_.portEdit->setText(QString("%1").arg(ports[index]));
}

void siteDialog::setParamFromUI() {
  param_.name_ = ui_.nameEdit->text();
  param_.hostAddress_ = ui_.addrEdit->text();
  param_.protocolType_ = ui_.protocolCombo->currentIndex();
  bool ok;
  int port = ui_.portEdit->text().toInt(&ok);
  param_.port_ = (ok && port >= 0 && port <= 65535)?port:ports[ui_.protocolCombo->currentIndex()];
  param_.hostType_ = ui_.hostTypeCombo->currentIndex();
  if (!ui_.proxyCheck->isChecked()) {
    param_.proxyType_ = 0;
  }
  param_.isAutoLogin_ = ui_.autoLoginCheck->isChecked();
}

//auto fix errors.
void siteDialog::setUIFromParam() {
  ui_.nameEdit->setText(param_.name_);
  ui_.addrEdit->setText(param_.hostAddress_);
  ui_.protocolCombo->setCurrentIndex(param_.protocolType_);
  ui_.portCheck->setCheckState(param_.port_ == ports[ui_.protocolCombo->currentIndex()]?Qt::Unchecked:Qt::Checked);
  ui_.portEdit->setText(QString("%1").arg(param_.port_));
  ui_.hostTypeCombo->setCurrentIndex(param_.hostType_);
  ui_.proxyCheck->setCheckState(param_.proxyType_?Qt::Checked:Qt::Unchecked);
  ui_.autoLoginCheck->setCheckState(param_.isAutoLogin_?Qt::Checked:Qt::Unchecked);

  previewFont();
}

void siteDialog::loadCurrentParam() {
  if (ui_.siteList->count() == 0) {
    return;
  }
  loadAddress(config_, currentSiteIndex(), param_);
}

void siteDialog::saveCurrentParam() {
  if (ui_.siteList->count() == 0) {
    return;
  }
  saveAddress(config_, currentSiteIndex(), param_);
}

void siteDialog::previewFont() {
  //issue 98

  QPalette palette;
  palette.setColor(QPalette::Window, param_.backgroundColor_);
  palette.setColor(QPalette::WindowText, param_.foregroundColor_);
  ui_.fontPreviewer->setPalette(palette);

  QString sample("<html><body style=\" font-family:'" 
    + param_.englishFontName_ + "'; font-size:" 
    + QString().setNum(param_.englishFontSize_) 
    + "pt;\"><BR>AaBbCc</body></html>");
  sample += QString("<html><body style=\" font-family:'"
    + param_.otherFontName_ + "'; font-size:" 
    + QString().setNum(param_.otherFontSize_) + "pt;\">"
    + param_.otherFontName_ + "<BR></body></html>");
  ui_.fontPreviewer->setText(sample);
}

void siteDialog::swapSite(int first, int second) {
  FQTermParam firstParam;
  FQTermParam secondParam;
  loadAddress(config_, first, firstParam);
  loadAddress(config_, second, secondParam);
  saveAddress(config_, first, secondParam);
  saveAddress(config_, second, firstParam);

  QListWidgetItem* firstItem = ui_.siteList->item(first);
  QListWidgetItem* secondItem = ui_.siteList->item(second);
  QString tmpStr = firstItem->text();
  firstItem->setText(secondItem->text());
  secondItem->setText(tmpStr);
}

int siteDialog::moveSite(int pos, int offset)
{
  int newPos = pos + offset;
  int step = offset>0?1:-1;
  if (newPos >= ui_.siteList->count() || newPos < 0) {
    return pos;
  }
  for(int i = pos; i != newPos; i += step) {
    swapSite(i , i + step);
  }
  return newPos;
}

QMessageBox::StandardButton siteDialog::checkModification(int row) { //with current display
  FQTermParam originParam;
  loadAddress(config_, row, originParam);
  setParamFromUI();
  QMessageBox::StandardButton ret = QMessageBox::No;
  if (!(originParam == param_)) {
    QMessageBox msgBox;
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(tr("Warning"));
    msgBox.setText(tr("The site configuration has been changed.\nTo save, press Yes.\nTo discard, press No.\nTo continue editing, press Cancel"));


    switch(ret = static_cast<QMessageBox::StandardButton>(msgBox.exec())) {
      case QMessageBox::No:
        param_ = originParam;
        break;
      default:
        break;
    }
    ui_.siteList->item(row)->setText(param_.name_);
  }
  return ret;
}

void siteDialog::onSelectSite(QListWidgetItem* current, QListWidgetItem* previous)
{
  int currentRow = ui_.siteList->row(current);
  int previousRow = ui_.siteList->row(previous);
  if (currentRow < 0) {
    ui_.editGroup->setDisabled(true);
  }
  else {
    ui_.editGroup->setEnabled(true);
  }
  if (previousRow >= 0){
    if (checkModification(previousRow) == QMessageBox::Cancel) {
      forcedSetCurrentSite(previousRow);
      return;
    }
    saveAddress(config_, previousRow, param_);    
  }
  if (currentRow >= 0){
    loadAddress(config_, currentRow, param_);
    setUIFromParam();
  }
  
}

//without emit a signal
void siteDialog::forcedSetCurrentSite(int row)
{
  FQ_VERIFY(disconnect(ui_.siteList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
    this, SLOT(onSelectSite(QListWidgetItem*, QListWidgetItem*))));
  ui_.siteList->setCurrentRow(row);
  FQ_VERIFY(connect(ui_.siteList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
    this, SLOT(onSelectSite(QListWidgetItem*, QListWidgetItem*))));
}

//emit a signal
void siteDialog::setCurrentSite(int row)
{
  ui_.siteList->setCurrentRow(row);
}

void siteDialog::removeSite(int row)
{
  if (row < 0) {
    return;
  }
  int last = ui_.siteList->count() - 1;
  setCurrentSite(last - 1);
  moveSite(row, last - row);
  config_->deleteSection(QString("%1").arg(last));
  config_->setItemValue("bbs list", "num", QString("%1").arg(last));
  delete ui_.siteList->takeItem(last);

  loadCurrentParam();
  setUIFromParam();
}

void siteDialog::onNew() {
  int row = ui_.siteList->count();
  if (row < 0) {
    row = 0;
  }
  config_->setItemValue("bbs list", "num", QString("%1").arg(row + 1));

  FQTermParam newParam;
  loadAddress(config_, -1, newParam);
  saveAddress(config_, row, newParam);
  ui_.siteList->addItem(newParam.name_);  
  setCurrentSite(row);
}

void siteDialog::onDelete() {
  if (ui_.siteList->count() == 0) {
    return;
  }
  QMessageBox msgBox;
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setIcon(QMessageBox::Warning);
  msgBox.setWindowTitle(tr("Warning"));
  msgBox.setText(tr("Are you sure to DELETE this site?"));
  switch(msgBox.exec())
  {
  case QMessageBox::Yes:
    removeSite(currentSiteIndex());
    break;
  case QMessageBox::No:
    break;
  }
}

void siteDialog::siteNameChanged( QString name ) {
  QListWidgetItem* item = ui_.siteList->currentItem();
  if (item) {
    item->setText(name);
  }
}

void siteDialog::setSiteSelected() {
  int row = currentSiteIndex();
  if(row >= 0) {
    ui_.siteList->item(row)->setSelected(true);
  }
}

void siteDialog::close(int doneValue) {
  if (ui_.siteList->count() > 0) {
    switch(checkModification(currentSiteIndex())) {
      case QMessageBox::Yes:
        onApply();
        break;
      case QMessageBox::No:
        loadCurrentParam();
        break;
      case QMessageBox::Cancel:
        return;
      default:
        break;
    }
  }
  done(doneValue);
}

void siteDialog::onReset() {
  loadCurrentParam();
  setUIFromParam();
}

void siteDialog::onApply() {
  setParamFromUI();
  saveCurrentParam();
  setUIFromParam();
}

void siteDialog::onUp() {
  int newPos = moveSite(currentSiteIndex(), -1);
  forcedSetCurrentSite(newPos);
}

void siteDialog::onDown() {
  int newPos = moveSite(currentSiteIndex(), 1);
  forcedSetCurrentSite(newPos);
}

void siteDialog::onClose() {
  close(0);
}

void siteDialog::onConnect() {
  close(1);
}

void siteDialog::resizeEvent( QResizeEvent * re ) {
  previewFont();
}

FQTermParam siteDialog::currentParameter() {
  return param_;
}

int siteDialog::currentSiteIndex() {
  return ui_.siteList->currentRow();
}

void siteDialog::onAdvance() {
  setting(addrDialog::General);
}

void siteDialog::setting(addrDialog::Tabs tab) {
  setParamFromUI();
  addrDialog addr(this, param_, addrDialog::APPLY);
  addr.setCurrentTabIndex(tab);
  int res = addr.exec();
  if (res == 1) {
    param_ = addr.param();
    setUIFromParam();
  }
}

void siteDialog::onProxy() {
  setting(addrDialog::Proxy);
}

void siteDialog::onAutoLogin() {
  setting(addrDialog::General);
}

void siteDialog::onDblClicked(QListWidgetItem * item) {
  onConnect();
}

void siteDialog::connector() {
  FQ_VERIFY(connect(ui_.siteList, SIGNAL(itemSelectionChanged()),
    this, SLOT(setSiteSelected())));
  FQ_VERIFY(connect(ui_.siteList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
    this, SLOT(onSelectSite(QListWidgetItem*, QListWidgetItem*))));
  FQ_VERIFY(connect(ui_.nameEdit, SIGNAL(textChanged(QString)),
    this, SLOT(siteNameChanged(QString))));
  FQ_VERIFY(connect(ui_.protocolCombo, SIGNAL(currentIndexChanged(int)), 
    this, SLOT(onSelectProtocol(int))));
  FQ_VERIFY(connect(ui_.resetButton, SIGNAL(clicked()),
    this, SLOT(onReset())));
  FQ_VERIFY(connect(ui_.upButton, SIGNAL(clicked()),
    this, SLOT(onUp())));
  FQ_VERIFY(connect(ui_.downButton, SIGNAL(clicked()),
    this, SLOT(onDown())));
  FQ_VERIFY(connect(ui_.newButton, SIGNAL(clicked()),
    this, SLOT(onNew())));
  FQ_VERIFY(connect(ui_.deleteButton, SIGNAL(clicked()),
    this, SLOT(onDelete())));
  FQ_VERIFY(connect(ui_.applyButton, SIGNAL(clicked()),
    this, SLOT(onApply())));
  FQ_VERIFY(connect(ui_.closeButton, SIGNAL(clicked()),
    this, SLOT(onClose())));
  FQ_VERIFY(connect(ui_.connectButton, SIGNAL(clicked()),
    this, SLOT(onConnect())));
  FQ_VERIFY(connect(ui_.advanceButton, SIGNAL(clicked()),
    this, SLOT(onAdvance())));
  FQ_VERIFY(connect(ui_.proxyButton, SIGNAL(clicked()),
    this, SLOT(onProxy())));
  FQ_VERIFY(connect(ui_.autoLoginButton, SIGNAL(clicked()),
    this, SLOT(onAutoLogin())));
  FQ_VERIFY(connect(ui_.siteList, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
    this, SLOT(onDblClicked(QListWidgetItem *))));
}

} //namespace FQTerm

#include "sitemanager.moc"
