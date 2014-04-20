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

#ifndef FQTERM_SCHEMADIALOG_H
#define FQTERM_SCHEMADIALOG_H

#include "ui_schemadialog.h"
#include <QStringList>
namespace FQTerm {

class schemaDialog: public QDialog {
  Q_OBJECT;
 public:
  schemaDialog(QWidget *parent = 0, Qt::WindowFlags fl = 0);
  ~schemaDialog();

  static QFileInfoList getSchemaList();
 protected:
  QColor colors[16];
  QPushButton * colorButtons[16];
  QString title_;
  QStringList fileList_;

  bool isModified_;
  int lastItemID_;
 private:
  Ui::schemaDialog ui_;
  QButtonGroup buttonGroup_;

 protected:
  void connectSlots();

  void loadList();
  void loadSchema(const QString &schemaFileName);
  int saveNumSchema(int n = -1);

  void updateView();

 protected slots:
  void buttonClicked();
  void nameChanged(int);
  void chooseImage();

  void saveSchema();
  void removeSchema();

  void onOK();
  void onCancel();

  void modified(const QString&) {modified();}
  void modified(int) {modified();}
  void modified(bool) {modified();}
  void modified() {isModified_ = true;}

  void clearModified() {isModified_ = false;}

signals:
  void schemaEdited();
};

}  // namespace FQTerm

#endif  // FQTERM_SCHEMADIALOG_H
