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

#ifndef FQTERM_SHORTCUT_DIALOG_H
#define FQTERM_SHORTCUT_DIALOG_H
#include <QDialog>
#include <QLineEdit>
class QTableWidget;
namespace FQTerm
{
class FQTermShortcutHelper;

class FQTermShortcutTableWidget : public QLineEdit
{
public:
  FQTermShortcutTableWidget(QWidget* parent) : QLineEdit(parent) {
  }
protected:
  void keyReleaseEvent(QKeyEvent * event);
  void keyPressEvent(QKeyEvent * event);
};

class FQTermShortcutDialog : public QDialog
{
  Q_OBJECT; 
public:
  FQTermShortcutDialog(FQTermShortcutHelper* helper, QWidget *parent_ = 0, Qt::WindowFlags fl = Qt::Dialog);
  ~FQTermShortcutDialog();
private:
  FQTermShortcutHelper* helper_;
  QTableWidget* table_;
protected slots:
  void defaultClicked(int row);
  void okBtnClicked();
  void applyBtnClicked();
  void cancelBtnClicked();
  void resetBtnClicked();
private:
  void applyChanges();
};
}//namespace FQTerm


#endif //FQTERM_SHORTCUT_DIALOG_H
