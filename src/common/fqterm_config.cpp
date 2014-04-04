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

#include <QTextStream>
#include <QFile>
#include <QString>
#include <QTextCodec>

#include "fqterm_trace.h"
#include "fqterm_config.h"

namespace FQTerm {

FQTermConfig::FQTermConfig(const QString &szFileName) {
  load(szFileName);
}

FQTermConfig::~FQTermConfig(){

}

bool FQTermConfig::save(const QString &szFileName) {
  QFile file(szFileName);
  if (!file.open(QIODevice::WriteOnly)) {
    FQ_TRACE("config", 0) << "Failed to open the file for writing: "
                          << szFileName;
    return false;
  }

  QTextStream os;
  os.setDevice(&file);
  saveToStream(os);
  // 	os.unsetDevice();
  file.close();


  return true;
}

bool FQTermConfig::loadFromStream(QTextStream &is) {
  QString strLine, strSection;

  data.clear();

  is.setCodec(QTextCodec::codecForName("UTF-8"));

  while (!is.atEnd()) {
    strLine = is.readLine().trimmed();
    if (strLine.isEmpty() || strLine[0] == '#') {
      continue;
    }

    if (strLine.left(1) == "[" && strLine.right(1) == "]") {
      strSection = strLine.mid(1, strLine.length() - 2);
      addSection(strSection);
    } else {
      QString strValue = strLine.section('=', 1).trimmed();
      setItemValue(strSection, strLine.section('=', 0, 0).trimmed(),
                   strValue.isNull() ? QString(""): strValue);
    }
  }
  return true;
}

bool FQTermConfig::saveToStream(QTextStream &os) {
  QString strLine, strSection;
  Section::iterator iStr;

  os.setCodec(QTextCodec::codecForName("UTF-8"));

  for (StrSecMap::iterator iSec = data.begin(); iSec != data.end(); ++iSec) {
    os << '[' << iSec.key() << "]\n";
    for (iStr = iSec.value().begin(); iStr != iSec.value().end(); ++iStr) {
      os << iStr.key() << '=' << iStr.value() << '\n';
    }
    os << '\n';
  }
  return true;
}


bool FQTermConfig::addSection(const QString &szSection) {
  if (hasSection(szSection)) {
    return false;
  }
  Section sec;
  data[szSection] = sec;
  return true;
}

bool FQTermConfig::hasSection(const QString &szSection) const 
{
  return data.find(szSection) != data.end();
}

bool FQTermConfig::setItemValue(const QString &szSection, const QString
                                &szItemName, const QString &szItemValue) {
  if (!hasSection(szSection))
    if (!addSection(szSection)) {
      return false;
	}

  data[szSection][szItemName] = szItemValue;
  return true;
}

QString FQTermConfig::getItemValue(const QString &szSection,
                                   const QString &szItemName) const {
  if (hasSection(szSection))
    if (data[szSection].find(szItemName) != data[szSection].end())
      if (!data[szSection][szItemName].isEmpty()) {
        return data[szSection][szItemName];
      }
  return "";
}

bool FQTermConfig::renameSection(const QString &szSection, const QString
                                 &szNewName) {
  if (hasSection(szNewName) || !hasSection(szSection)) {
    return false;
  }

  if (!addSection(szNewName)) {
    return false;
  }
  data[szNewName] = data[szSection];

  return deleteSection(szSection);
}

bool FQTermConfig::deleteSection(const QString &szSection) {
  if (hasSection(szSection)) {
    data.remove(szSection);
    return true;
  }
  return false;
}

bool FQTermConfig::deleteItem(const QString &szSection, const QString
                              &szItemName) {
  if (hasSection(szSection))
	if (data[szSection].find(szItemName) != data[szSection].end()) {
      data[szSection].remove(szItemName);
      return true;
	}
  return false;
}

bool FQTermConfig::load(const QString &filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    FQ_TRACE("config", 0) << "Failed to open the file for reading "
                          << filename;
    return false;
  }
  QTextStream is;
  is.setDevice(&file);
  loadFromStream(is);
  //is.unsetDevice();
  file.close();
  return true;
}

}  // namespace FQTerm
