// SPDX-License-Identifier: GPL-2.0-or-later

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
