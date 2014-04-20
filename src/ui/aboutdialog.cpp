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

#include <QPixmap>
#include <QFile>
#include <QTextBrowser>
#include <QTextStream>
#include <QtGlobal>

#include "fqterm.h"
#include "fqterm_path.h"
#include "aboutdialog.h"

namespace FQTerm {

/*
 *  Constructs a aboutDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
aboutDialog::aboutDialog(QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl) {
  ui_.setupUi(this);

  QFile file(getPath(RESOURCE) + "credits");
  if (file.open(QIODevice::ReadOnly)) {
    QTextStream stream(&file);
    QString line;
    while (!stream.atEnd()) {
      // line of text excluding '\n'
      line += stream.readLine() + "\n";
    }
    ui_.TextBrowser->setPlainText(line);
    file.close();
  }
  QString aboutText = "FQTerm "  + QString(FQTERM_VERSION_STRING) + QString("\ngit revision ") + QString(FQTERM_GIT_REVISION) + QString("\n Built with Qt") + QT_VERSION_STR + QString("\n Running with Qt ") + qVersion();
   ui_.TextLabel->setText(aboutText);
}

/*
 *  Destroys the object and frees any allocated resources
 */
aboutDialog::~aboutDialog() {
  // no need to delete child widgets, Qt does it all for us
}

}  // namespace FQTerm

#include "aboutdialog.moc"
