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

#ifndef FQTERM_QUICK_DIALOG_H
#define FQTERM_QUICK_DIALOG_H

#include "fqterm_param.h"
#include "ui_quickdialog.h"
//Added by qt3to4:
//#include <QCloseEvent>

class QCloseEvent;
class QResizeEvent;

namespace FQTerm {

class FQTermConfig;

class quickDialog: public QDialog {
  Q_OBJECT;
 public:
  quickDialog(FQTermConfig *, QWidget *parent_ = 0, Qt::WindowFlags fl = 0);
  ~quickDialog();

  FQTermParam param_;

  static const int ports[];

 protected slots:
  void listChanged(int);  //
  void onAdd();
  void onDelete();
  void onAdvance();
  void onConnect(); //
  void onClose();
  void onSelectProtocol(int index);
  void portCheckToggled(bool);

 private:

  void closeEvent(QCloseEvent*);  //
  void connectSlots();
  void loadHistory(); //
  void setUIFromParam();
  void setParamFromUI();
  void loadParam(int index);
  void saveParam(int index);

  FQTermConfig *config_;

 private:
  Ui::quickDialog ui_;
};

}  // namespace FQTerm

#endif // FQTERM_QUICK_DIALOG_H
