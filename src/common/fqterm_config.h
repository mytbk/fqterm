// SPDX-License-Identifier: GPL-2.0-or-later

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
