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

#ifndef FQTERM_FILEDIALOG_H
#define FQTERM_FILEDIALOG_H

#include <QFileDialog>
#include <QMessageBox>

namespace FQTerm {

class FQTermConfig;

class FQTermFileDialog: public QFileDialog {
public:
  FQTermFileDialog(FQTermConfig *config, QWidget *parent_ = 0, Qt::WFlags fl = 0);
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
