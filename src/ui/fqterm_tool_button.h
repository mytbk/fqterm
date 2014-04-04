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

#ifndef FQTERM_TOOL_BUTTON_H
#define FQTERM_TOOL_BUTTON_H

#include <QToolButton>

namespace FQTerm {

class FQTermToolButton: public QToolButton {
  Q_OBJECT;
 public:
  FQTermToolButton(QWidget *parent_, int id, QString name = "");
  ~FQTermToolButton();

 signals:
  void buttonClicked(int);

 protected slots:
  void slotClicked();

 protected:
  int id_;
};

}  // namespace FQTerm

#endif // FQTERM_TOOL_BUTTON_H
