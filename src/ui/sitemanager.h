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

#ifndef FQTERM_SITEMANGER_H
#define FQTERM_SITEMANGER_H

#include "addrdialog.h"

#include "ui_sitemanager.h"
#include "ui_quickdialog.h"

class QListWidgetItem;

namespace FQTerm {
class FQTermParam;
class FQTermConfig;

class siteDialog: public QDialog {
  Q_OBJECT;
 public:
  siteDialog(QWidget *parent_ = 0, Qt::WindowFlags fl = 0);;
  ~siteDialog();

  FQTermParam currentParameter();

  int currentSiteIndex();

 private:
  void connector();

  void resizeEvent(QResizeEvent * re);

  //Note: changes are made on config_
  void saveCurrentParam();
  void loadCurrentParam();

  void setParamFromUI();
  void setUIFromParam();

  void previewFont();

  void swapSite(int first, int second);
  int moveSite(int pos, int offset);
  void forcedSetCurrentSite(int row);
  void setCurrentSite(int row);

  QMessageBox::StandardButton checkModification(int row);

  void removeSite(int row);

  void close(int doneValue);
  void setting(addrDialog::Tabs tab);


 protected slots:
  void onSelectSite(QListWidgetItem* current, QListWidgetItem* previous);
  void siteNameChanged(QString name);
  void setSiteSelected();

  void onSelectProtocol(int index);

  void onReset();
  void onApply();
  void onUp();
  void onDown();
  void onNew();
  void onDelete();
  void onClose();
  void onConnect();

  void onAdvance();
  void onProxy();
  void onAutoLogin();
  void onDblClicked(QListWidgetItem * item);

 private:
  FQTermConfig *config_;
  FQTermParam param_;

  Ui::siteManager ui_;

  static int ports[]; //telnet, ssh1, ssh2
};

}  //FQTerm namespace

#endif  // FQTERM_SITEMANGER_H
