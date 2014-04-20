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

#include <QFileDialog>

#include "fqterm_config.h"
#include "fqterm_path.h"
#include "fqterm_trace.h"

#include "keydialog.h"

namespace FQTerm {

/*
 *  Constructs a keyDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
keyDialog::keyDialog(FQTermConfig * config, QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl),
      keyButtonGroup_(this),
      config_(config) {
  ui_.setupUi(this);

  keyButtonGroup_.addButton(ui_.radioButton1, 0);
  keyButtonGroup_.addButton(ui_.radioButton2, 1);
  keyButtonGroup_.addButton(ui_.radioButton3, 2);

  connectSlots();

  loadName();
}

keyDialog::~keyDialog() {
  // no need to delete child widgets, Qt does it all for us
}


void keyDialog::connectSlots() {  
  FQ_VERIFY(connect(ui_.nameListWidget, SIGNAL(currentRowChanged(int)),
                  this, SLOT(onNamechange(int))));
  FQ_VERIFY(connect(ui_.addButton, SIGNAL(clicked()), this, SLOT(onAdd())));
  FQ_VERIFY(connect(ui_.deleteButton, SIGNAL(clicked()), this, SLOT(onDelete())));
  FQ_VERIFY(connect(ui_.updateButton, SIGNAL(clicked()), this, SLOT(onUpdate())));
  FQ_VERIFY(connect(ui_.closeButton, SIGNAL(clicked()), this, SLOT(onClose())));
  FQ_VERIFY(connect(ui_.upButton, SIGNAL(clicked()), this, SLOT(onUp())));
  FQ_VERIFY(connect(ui_.downButton, SIGNAL(clicked()), this, SLOT(onDown())));
  FQ_VERIFY(connect(ui_.leftButton, SIGNAL(clicked()), this, SLOT(onLeft())));
  FQ_VERIFY(connect(ui_.rightButton, SIGNAL(clicked()), this, SLOT(onRight())));
  FQ_VERIFY(connect(ui_.enterButton, SIGNAL(clicked()), this, SLOT(onEnter())));
//  FQ_VERIFY(connect(&keyButtonGroup_, SIGNAL(buttonClicked(int)), this, SLOT(onSelect(int))));
  FQ_VERIFY(connect(ui_.scriptButton, SIGNAL(clicked()), this, SLOT(onChooseScript())));
  FQ_VERIFY(connect(ui_.programButton, SIGNAL(clicked()), this, SLOT(onProgram())));

}

void keyDialog::onNamechange(int item) {
  loadKey(item);
}

void keyDialog::onAdd() {
  QString strTmp = config_->getItemValue("key", "num");
  int num = strTmp.toInt();

  strTmp.setNum(num + 1);
  config_->setItemValue("key", "num", strTmp);

  QString strValue;
  switch (keyButtonGroup_.checkedId()) {
    case 0:
      strValue = "0" + ui_.keyEdit->text();
      break;
    case 1:
      strValue = "1" + ui_.scriptEdit->text();
      break;
    case 2:
      strValue = "2" + ui_.programEdit->text();
      break;
  }
  strTmp = QString("key%1").arg(num);
  config_->setItemValue("key", strTmp, strValue);

  strTmp = QString("name%1").arg(num);
  config_->setItemValue("key", strTmp, ui_.nameEdit->text());

  strTmp = QString("shortcut%1").arg(num);
  config_->setItemValue("key", strTmp, ui_.shortcutEdit->text());

  ui_.nameListWidget->addItem(ui_.nameEdit->text());
  ui_.nameListWidget->setCurrentRow(ui_.nameListWidget->count() -1);
}

void keyDialog::onDelete() {
  QString strTmp = config_->getItemValue("key", "num");
  int num = strTmp.toInt();
  if (num == 0) {
    return ;
  }
  strTmp.setNum(num -1);
  config_->setItemValue("key", "num", strTmp);

  int index = ui_.nameListWidget->currentRow();
  QString strItem1, strItem2;
  for (int i = index; i < num -1; i++) {
    strItem1 = QString("key%1").arg(i);
    strItem2 = QString("key%1").arg(i + 1);
    config_->setItemValue("key", strItem1, config_->getItemValue("key", strItem2));
    strItem1 = QString("name%1").arg(i);
    strItem2 = QString("name%1").arg(i + 1);
    config_->setItemValue("key", strItem1, config_->getItemValue("key", strItem2));
  }

  ui_.nameListWidget->takeItem(index);
  ui_.nameListWidget->setCurrentRow(qMin(index, ui_.nameListWidget->count() -1));
}

void keyDialog::onUpdate() {
  int index = ui_.nameListWidget->currentRow();
  if (index < 0) {
    return ;
  }

  QString strValue;
  switch (keyButtonGroup_.checkedId()) {
    case 0:
      strValue = "0" + ui_.keyEdit->text();
      break;
    case 1:
      strValue = "1" + ui_.scriptEdit->text();
      break;
    case 2:
      strValue = "2" + ui_.programEdit->text();
      break;
  }

  QString strTmp;

  strTmp = QString("key%1").arg(index);
  config_->setItemValue("key", strTmp, strValue);

  strTmp = QString("name%1").arg(index);
  config_->setItemValue("key", strTmp, ui_.nameEdit->text());

  strTmp = QString("shortcut%1").arg(index);
  config_->setItemValue("key", strTmp, ui_.shortcutEdit->text());

  ui_.nameListWidget->item(index)->setText(ui_.nameEdit->text());
}

void keyDialog::onClose() {
  config_->save(getPath(USER_CONFIG) + "fqterm.cfg");
  done(1);
}

void keyDialog::onUp() {
  ui_.keyEdit->insert("^[[A");
}

void keyDialog::onDown() {
  ui_.keyEdit->insert("^[[B");
}

void keyDialog::onLeft() {
  ui_.keyEdit->insert("^[[D");
}

void keyDialog::onRight() {
  ui_.keyEdit->insert("^[[C");
}

void keyDialog::onEnter() {
  ui_.keyEdit->insert("^M");
}

// void keyDialog::onSelect(int id)
// {
// 	switch(id)
// 	{
// 		case 0:	// key
// 			ui.keyEdit->setEnabled(true);
// 			ui.scriptEdit->setEnabled(false);
// 			ui.scriptButton->setEnabled(false);
// 			ui.programEdit->setEnabled(false);
// 			ui.programButton->setEnabled(false);
// 			break;
// 		case 1:	// script
// 			ui.keyEdit->setEnabled(false);
// 			ui.scriptEdit->setEnabled(true);
// 			ui.scriptButton->setEnabled(true);
// 			ui.programEdit->setEnabled(false);
// 			ui.programButton->setEnabled(false);
// 		break;
// 		case 2:	// program
// 			ui.scriptEdit->setEnabled(false);
// 			ui.scriptButton->setEnabled(false);
// 			ui.keyEdit->setEnabled(false);
// 			ui.programEdit->setEnabled(true);
// 			ui.programButton->setEnabled(true);
// 			break;
// 	}
// }
void keyDialog::onChooseScript() {
  QString script = QFileDialog::getOpenFileName(
      this, "Select a script", QDir::currentPath(), "JavaScript File (*.js)");
  if (!script.isEmpty()) {
    ui_.scriptEdit->setText(script);
  }
}

void keyDialog::onProgram() {
  QString program = QFileDialog::getOpenFileName(
      this, "Select a program", QDir::currentPath(), "*");
  if (!program.isNull()) {
    ui_.programEdit->setText(program);
  }
}

void keyDialog::loadName() {
  QString strTmp = config_->getItemValue("key", "num");
  int num = strTmp.toInt();
  for (int i = 0; i < num; i++) {
    strTmp = QString("name%1").arg(i);
    ui_.nameListWidget->addItem(config_->getItemValue("key", strTmp));
  }
  if (num > 0) {
    ui_.nameListWidget->setCurrentRow(0);
  } else {
    ui_.radioButton1->setChecked(true);
    //onSelect(0);
  }
}

void keyDialog::loadKey(int n) {
  QString strTmp = config_->getItemValue("key", "num");
  if (n >= strTmp.toInt()) {
    return ;
  }

  QString strItem;

  strItem = QString("name%1").arg(n);
  ui_.nameEdit->setText(config_->getItemValue("key", strItem));

  strItem = QString("key%1").arg(n);
  strTmp = config_->getItemValue("key", strItem);
  if (strTmp[0] == '0') {
    ui_.keyEdit->setText(strTmp.mid(1));
    ui_.radioButton1->setChecked(true);
    //onSelect(0);
  } else if (strTmp[0] == '1') {
    ui_.scriptEdit->setText(strTmp.mid(1));
    ui_.radioButton2->setChecked(true);
    //onSelect(7);
  } else if (strTmp[0] == '2') {
    ui_.programEdit->setText(strTmp.mid(1));
    ui_.radioButton3->setChecked(true);
    //onSelect(6);
  }
  strItem = QString("shortcut%1").arg(n);
  strTmp = config_->getItemValue("key", strItem);
  ui_.shortcutEdit->setText(strTmp);
}

}  // namespace FQTerm

#include "keydialog.moc"
