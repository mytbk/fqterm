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

#include <stdio.h>

#include "msgdialog.h"

msgDialog::msgDialog(QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl) {
  ui_.setupUi(this);
  // signals and slots connections
  //QVERIFY(connect( okButton, SIGNAL( clicked() ), this, SLOT( accept() ) ));
}

/*  
 *  Destroys the object and frees any allocated resources
 */
msgDialog::~msgDialog() {
  // no need to delete child widgets, Qt does it all for us
}

void msgDialog::setMessageText(const QString& message) {
  ui_.msgBrowser->setPlainText(message);
}

#include "msgdialog.moc"
