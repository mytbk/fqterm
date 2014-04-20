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
#include <QColorDialog>
#include <QComboBox>
#include <QMessageBox>

#include "fqterm_config.h"
#include "fqterm_path.h"
#include "fqterm_trace.h"

#include "schemadialog.h"

namespace FQTerm {

schemaDialog::schemaDialog(QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl),
      buttonGroup_(this) {
  ui_.setupUi(this);
  buttonGroup_.addButton(ui_.noneRadioButton, 0);
  buttonGroup_.addButton(ui_.imageRadioButton, 1);

  lastItemID_ = -1;
  isModified_ = false;

  ui_.alphaSlider->setMinimum(0);
  ui_.alphaSlider->setMaximum(100);
  ui_.alphaSlider->setSingleStep(1);
  ui_.alphaSlider->setPageStep(10);

  // 	ui.bgButtonGroup->setRadioButtonExclusive(true);
  colorButtons[0] = ui_.clr0Button;
  colorButtons[1] = ui_.clr1Button;
  colorButtons[2] = ui_.clr2Button;
  colorButtons[3] = ui_.clr3Button;
  colorButtons[4] = ui_.clr4Button;
  colorButtons[5] = ui_.clr5Button;
  colorButtons[6] = ui_.clr6Button;
  colorButtons[7] = ui_.clr7Button;
  colorButtons[8] = ui_.clr8Button;
  colorButtons[9] = ui_.clr9Button;
  colorButtons[10] = ui_.clr10Button;
  colorButtons[11] = ui_.clr11Button;
  colorButtons[12] = ui_.clr12Button;
  colorButtons[13] = ui_.clr13Button;
  colorButtons[14] = ui_.clr14Button;
  colorButtons[15] = ui_.clr15Button;

  connectSlots();
  loadList();
}


schemaDialog::~schemaDialog(){}


void schemaDialog::connectSlots() {
  FQ_VERIFY(connect(ui_.saveButton, SIGNAL(clicked()), this, SLOT(saveSchema())));
  FQ_VERIFY(connect(ui_.removeButton, SIGNAL(clicked()), this, SLOT(removeSchema())));
  FQ_VERIFY(connect(ui_.okButton, SIGNAL(clicked()), this, SLOT(onOK())));
  FQ_VERIFY(connect(ui_.cancelButton, SIGNAL(clicked()), this, SLOT(onCancel())));

  for (int i = 0; i < 16; ++i) {
    FQ_VERIFY(connect(colorButtons[i], SIGNAL(clicked()), this, SLOT(buttonClicked())));
  }

  FQ_VERIFY(connect(ui_.nameListWidget, SIGNAL(currentRowChanged(int)),
                  this, SLOT(nameChanged(int))));


  FQ_VERIFY(connect(ui_.chooseButton, SIGNAL(clicked()), 
                  this, SLOT(chooseImage())));

  FQ_VERIFY(connect(ui_.titleLineEdit, SIGNAL(textChanged(const QString &)),
                  this, SLOT(modified(const QString &))));
  FQ_VERIFY(connect(ui_.imageLineEdit, SIGNAL(textChanged(const QString &)),
                  this, SLOT(modified(const QString &))));
  FQ_VERIFY(connect(ui_.alphaSlider, SIGNAL(valueChanged(int)), 
                  this, SLOT(modified(int))));
  FQ_VERIFY(connect(ui_.optionComboBox, SIGNAL(activated(int)), 
                  this, SLOT(modified(int))));
  FQ_VERIFY(connect(ui_.coverComboBox, SIGNAL(activated(int)), 
                  this, SLOT(modified(int))));
  FQ_VERIFY(connect(ui_.noneRadioButton, SIGNAL(toggled(bool)), 
                  this, SLOT(modified(bool))));
  FQ_VERIFY(connect(ui_.imageRadioButton, SIGNAL(toggled(bool)), 
                  this, SLOT(modified(bool))));
  FQ_VERIFY(connect(ui_.alphaCheckBox, SIGNAL(toggled(bool)), 
                  this, SLOT(modified(bool))));
}

void schemaDialog::loadList() {
  QFileInfoList lstFile = getSchemaList();

  //if(lstFile != NULL)
  {
    foreach(QFileInfo fi, lstFile) {
      FQTermConfig *pConf = new FQTermConfig(fi.absoluteFilePath());
      QListWidgetItem* item = new QListWidgetItem(pConf->getItemValue("schema", "title"), ui_.nameListWidget);
      item->setData(Qt::UserRole, fi.absoluteFilePath());
      ui_.nameListWidget->addItem(item);
      delete pConf;
    }
  }
  if (ui_.nameListWidget->count() != 0) {
    ui_.nameListWidget->setCurrentRow(0);
  }
}

void schemaDialog::loadSchema(const QString &strSchemaFile) {
  FQTermConfig *pConf = new FQTermConfig(strSchemaFile);

  title_ = pConf->getItemValue("schema", "title");
  //0 -- none 1 -- image
  int type = pConf->getItemValue("background", "type").toInt();
  if (type == 0) {
    ui_.noneRadioButton->setChecked(true);
  } else if (type == 1) {
    ui_.imageRadioButton->setChecked(true);
  }
  ui_.imageLineEdit->setText(pConf->getItemValue("image", "name"));
  QString strTmp = pConf->getItemValue("image", "render");
  ui_.optionComboBox->setCurrentIndex(strTmp.toInt());
  strTmp = pConf->getItemValue("image", "cover");
  ui_.coverComboBox->setCurrentIndex(strTmp.toInt());
  ui_.alphaCheckBox->setChecked(pConf->getItemValue("image", "usealpha").toInt());
  ui_.alphaSlider->setValue(pConf->getItemValue("image", "alpha").toInt());

  for (int i = 0; i < 16; ++i) {
    colors[i].setNamedColor(pConf->getItemValue("color", QString("color%1").arg(i)));
  }

  delete pConf;

  updateView();

}

QFileInfoList schemaDialog::getSchemaList() {
  QDir dir;
  dir.setNameFilters(QStringList("*.schema"));
  dir.setPath(getPath(USER_CONFIG) + "schema");
  return dir.entryInfoList();
}

int schemaDialog::saveNumSchema(int n) {

  int saved = n;
  title_ = ui_.titleLineEdit->text();

  QString schemaFileName = getPath(USER_CONFIG) + "schema/" + title_ + ".schema";
  QListWidgetItem* item = ui_.nameListWidget->currentItem();
  // create a new schema if title changed
  QString test = ui_.nameListWidget->currentItem()->data(Qt::UserRole).toString();
  if (QFileInfo(schemaFileName) != QFileInfo(ui_.nameListWidget->item(n)->data(Qt::UserRole).toString())) {
    item = new QListWidgetItem(title_, ui_.nameListWidget);
    item->setData(Qt::UserRole, schemaFileName);
    ui_.nameListWidget->addItem(item);
    saved = ui_.nameListWidget->row(item);
  }

  FQTermConfig *pConf = new FQTermConfig(ui_.nameListWidget->item(n)->data(Qt::UserRole).toString());

  pConf->setItemValue("schema", "title", title_);

  QString strTmp;
  //0 -- none 1 -- image
  int type = 0;
  
  if (ui_.noneRadioButton->isChecked()) {
    type = 0;
  } else if (ui_.imageRadioButton->isChecked()) {
    type = 1;
  }
  strTmp.setNum(type);
  pConf->setItemValue("background", "type", strTmp);

  pConf->setItemValue("image", "name", ui_.imageLineEdit->text());


  strTmp.setNum(ui_.optionComboBox->currentIndex());
  pConf->setItemValue("image", "render", strTmp);

  strTmp.setNum(ui_.coverComboBox->currentIndex());
  pConf->setItemValue("image", "cover", strTmp);
  
  pConf->setItemValue("image", "usealpha", ui_.alphaCheckBox->isChecked() ? "1" : "0");

  strTmp.setNum(ui_.alphaSlider->value());
  pConf->setItemValue("image", "alpha", strTmp);


  for (int i = 0; i < 16; ++i) {
    pConf->setItemValue("color", QString("color%1").arg(i), colors[i].name());
  }

  pConf->save(schemaFileName);

  delete pConf;

  clearModified();
  emit schemaEdited();
  return saved;
}

void schemaDialog::updateView() {
  // title
  ui_.titleLineEdit->setText(title_);
  for (int i = 0; i < 16; ++i) {
    QPalette palette;
    palette.setColor(QPalette::Button, colors[i]);
    colorButtons[i]->setPalette(palette);
  }

  // load from file, nothing changed
  clearModified();
}

void schemaDialog::buttonClicked() {
  QPushButton *button = (QPushButton*)sender();
  QColor color =
      QColorDialog::getColor(button->palette().color(button->backgroundRole()));
  if (color.isValid() == true) {
    QPalette palette;
    palette.setColor(QPalette::Button, color);
    button->setPalette(palette);
    modified();
  }
  for (int i = 0; i < 16; ++i) {
    if (colorButtons[i] == button) {
      colors[i] = color;
      break;
    }
  }
}

void schemaDialog::nameChanged(int item) {
  if (isModified_) {
    QMessageBox mb("FQTerm", "Setting changed, do you want to save?",
                   QMessageBox::Warning, QMessageBox::Yes | QMessageBox::Default,
                   QMessageBox::No | QMessageBox::Escape, 0, this);
    if (mb.exec() == QMessageBox::Yes) {
      if (lastItemID_ != -1) {
        saveNumSchema(lastItemID_);
      }
    }
  }

  int n = item; //nameListBox->index(item);
  lastItemID_ = n;


  loadSchema(ui_.nameListWidget->item(n)->data(Qt::UserRole).toString());
  updateView();
  ui_.nameListWidget->setCurrentRow(n, QItemSelectionModel::Select);
}


void schemaDialog::chooseImage() {
  QString img = QFileDialog::getOpenFileName(
      this, "Choose an image", QDir::currentPath());
  if (!img.isNull()) {
    ui_.imageLineEdit->setText(img);
    isModified_ = true;
  }
}



void schemaDialog::saveSchema() {
  // get current schema file name
  int n = ui_.nameListWidget->currentRow();
  int saved = saveNumSchema(n);
  lastItemID_ = saved;
  ui_.nameListWidget->setCurrentRow(saved, QItemSelectionModel::Select);
}

void schemaDialog::removeSchema() {
  QFileInfo fi(ui_.nameListWidget->currentItem()->data(Qt::UserRole).toString());
  if (fi.isWritable()) {
    QFile::remove(ui_.nameListWidget->currentItem()->data(Qt::UserRole).toString());
    int n = ui_.nameListWidget->currentRow();
    delete ui_.nameListWidget->takeItem(n);
    emit schemaEdited();
  } else {
    QMessageBox::warning(this, "Error",
                         "This is a system schema. Permission Denied");
  }
}

void schemaDialog::onOK() {
  saveSchema();
  done(1);
}

void schemaDialog::onCancel() {
  done(0);
}




}  // namespace FQTerm

#include "schemadialog.moc"
