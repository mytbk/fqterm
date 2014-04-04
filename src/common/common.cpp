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

#include <stdlib.h>

#include <QString>
#include <QProcess>
#include <QStringList>
#include "common.h"

namespace FQTerm {

void runProgram(const QString &cmd, const QString& arg, bool bg) {
  if (bg) {
    QProcess::startDetached(cmd, QStringList(arg));
  } else {
    QProcess::execute(cmd, QStringList(arg));
  }
  
  return;
  /*
#if defined(WIN32)
  QString strCmd = "\"\"" + cmd + "\" \"" + arg + "\"\"";
#else
  QString strCmd = "'" + cmd + "' '" + arg + "'";
#endif
#if !defined(WIN32)
  if (bg) {
    strCmd += " &";
  }
#endif
  system(strCmd.toLocal8Bit());
  */
}

}  // namespace FQTerm
