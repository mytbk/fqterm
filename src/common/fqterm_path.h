// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_PATH_H
#define FQTERM_PATH_H

class QString;
class QStringList;

class FQTermConfig;
class FQTermParam;

namespace FQTerm {

extern QString *local_shell_bin;
enum PathCategory {RESOURCE, USER_CONFIG};

const QString &getPath(PathCategory category);

bool iniSettings();

void checkHelpExists(FQTermConfig*);
void loadNameList(FQTermConfig *, QStringList &);
bool loadAddress(FQTermConfig *, int, FQTermParam &);
void saveAddress(FQTermConfig *, int, const FQTermParam &);

void clearDir(const QString &path);
}  // namespace FQTerm

#endif  // FQTERM_PATH_H
