// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_FILEDIALOG_H
#define FQTERM_FILEDIALOG_H

#include <QFileDialog>
#include <QMessageBox>

namespace FQTerm {

class FQTermConfig;

class FQTermFileDialog: public QFileDialog {
public:
  FQTermFileDialog(FQTermConfig *config, QWidget *parent_ = 0, Qt::WindowFlags fl = 0);
  ~FQTermFileDialog();

  QString getSaveName(const QString &filename, const QString &hints, QWidget *widget = 0);
  QString getOpenName(const QString &title, const QString &hints, QWidget *widget = 0);
  QStringList getOpenNames(const QString &title, const QString &hints, QWidget *widget = 0);
  QString getExistingDirectory(const QString &title, const QString &hints, QWidget *widget = 0);

private:
  QString strSection, strSave, strOpen;
  FQTermConfig *config_;
  QString configDir, userConfig;
};

} // name space FQTerm

#endif  // FQTERM_FILEEDIALOG_H
