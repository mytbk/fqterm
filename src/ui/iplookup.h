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

#ifndef __FQTERM_IPLOOKUP__
#define __FQTERM_IPLOOKUP__

#include <QDialog>
#include "ui_iplookup.h"
namespace FQTerm {

class IPLookupDialog : public QDialog {
  Q_OBJECT;
public:
  IPLookupDialog(QWidget *parent_ = 0, Qt::WindowFlags fl = 0);
  ~IPLookupDialog();

protected slots:
  void onLookup();
  void onFinished();
private:
  Ui::IPLookupDialog ui_;
};

} //namespace FQTerm

#endif //__FQTERM_IPLOOKUP__