// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SCHEMADIALOG_H
#define FQTERM_SCHEMADIALOG_H

#include "ui_schemadialog.h"
#include <QStringList>
#include <QButtonGroup>

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
