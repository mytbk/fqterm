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

#ifndef FQTERM_CFG_H
#define FQTERM_CFG_H

#include <QTextStream>
#include <QMap>
#include <QObject>

namespace FQTerm {
typedef QMap<QString, QString> Section;
typedef QMap<QString, Section> StrSecMap;

class FQTermConfig {
 private:
  StrSecMap data;
  bool loadFromStream(QTextStream &);
  bool saveToStream(QTextStream &);

  bool addSection(const QString &sectionName);
 public:
  FQTermConfig(const QString &fileName);
  ~FQTermConfig();

  bool load(const QString &filename);
  bool save(const QString &fileName);

  bool setItemValue(const QString &sectionName, const QString &itemName,
                    const QString &itemValue);
  
  QString getItemValue(const QString &sectionName,
                       const QString &itemName) const;

  bool deleteItem(const QString &sectionName, const QString &itemName);
  bool deleteSection(const QString &sectionName);

  bool renameSection(const QString &sectionName, const QString &newName);
  bool hasSection(const QString &sectionName) const;
};

}  // namespace FQTerm

#endif  // FQTERM_CFG_H
