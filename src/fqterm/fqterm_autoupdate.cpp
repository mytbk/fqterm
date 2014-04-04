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

#include "fqterm_autoupdate.h"
#include "fqterm_config.h"
#include "fqterm_trace.h"
#include "fqterm_param.h"

#include <QDesktopServices>
#include <QFile>
#include <QHttp>
#include <QMessageBox>
#include <QString>
#include <QUrl>

namespace FQTerm {

////////////////////////////////////////////////
static int versionCompare(QString lhs, QString rhs) {
  //0 -- lhs == rhs, -1 -- lhs > rhs, 1 -- lhs < rhs
  
  int versionEnd = lhs.indexOf('-');
  if (versionEnd != -1) {
    lhs = lhs.mid(0, versionEnd);
  }
  versionEnd = rhs.indexOf('-');
  if (versionEnd != -1) {
    rhs = rhs.mid(0, versionEnd);
  }
  QStringList lhsList = lhs.split(".");
  QStringList rhsList = rhs.split(".");
  if (lhsList.size() != 3 || rhsList.size() != 3) {
    return 0; //something is wrong
  }
  for (int i = 0; i < 3; ++i) {
    if (lhsList[i].toInt() < rhsList[i].toInt()) {
      return 1;
    }
    else if (lhsList[i].toInt() > rhsList[i].toInt()) {
      return -1;
    }
  }
  return 0;
}
///////////////////////////////////////////////

void FQTermAutoUpdater::checkUpdate() {
  QString fileName = FQTermPref::getInstance()->poolDir_ + "VersionInfo";
  if (QFile::exists(fileName)) {
    QFile::remove(fileName);
  }
  versionInfoFile_ = new QFile(fileName, this);
  if (!versionInfoFile_->open(QIODevice::WriteOnly)) {
    delete versionInfoFile_;
    return;
  }
  FQ_VERIFY(connect(updateChecker_, SIGNAL(requestFinished(int, bool)),
    this, SLOT(checkRequestFinished(int, bool))));
  FQ_VERIFY(connect(updateChecker_, SIGNAL(done(bool)),
    this, SLOT(httpDone(bool))));

  const QString versionInfoURL = "http://fqterm.googlecode.com/svn/trunk/VersionInfo";
  QUrl url(versionInfoURL);
  QHttp::ConnectionMode mode = url.scheme().toLower() == "https" ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp;
  updateChecker_->setHost(url.host(), mode, url.port() == -1 ? 0 : url.port());
  if (!url.userName().isEmpty())
    updateChecker_->setUser(url.userName(), url.password());
  checkRequestId_ =  updateChecker_->get(url.path(), versionInfoFile_);
}

QString FQTermAutoUpdater::getNewestVersion() {
#if defined(WIN32)
  const QString platform = "FQWin";
#elif defined(__unix__) && !defined(__APPLE__)
  const QString platform = "FQLinux";
#else
  const QString platform = "FQMac";
#endif
  FQTermConfig versionInfo(FQTermPref::getInstance()->poolDir_ + "VersionInfo");
  QString ver = versionInfo.getItemValue(platform, "version");
  if (ver.isEmpty()) {
    return FQTERM_VERSION_STRING;
  }
  return ver;
}

void FQTermAutoUpdater::promptUpdate() {
  const QString updateURL = "http://code.google.com/p/fqterm/downloads/list";
  QMessageBox msgBox(QMessageBox::Information, tr("FQTerm Update Notifier"), 
    tr("FQTerm update available.\nPress OK to visit our download list,\nDiscard for a future prompt,\nIgnore for no more notification."),
    QMessageBox::Ok | QMessageBox::Discard | QMessageBox::Ignore);
  switch (msgBox.exec()) {
   case QMessageBox::Ok:
     QDesktopServices::openUrl(updateURL);
     break;
   case QMessageBox::Discard:
     break;
   case QMessageBox::Ignore:
     config_->setItemValue("global", "newestversion", getNewestVersion());
     config_->setItemValue("global", "updateprompt", "0");
     break;
  } 
}

void FQTermAutoUpdater::checkRequestFinished(int requestId, bool error){
  
  if (checkDone_ || requestId != checkRequestId_) {
    return;
  }
  if (!error) {
    checkDone_ = true;
  }
}

void FQTermAutoUpdater::httpDone(bool err)  {
  if (!err) {
    versionInfoFile_->flush();
    if (checkDone_) {
      QString newestVersionRecorded = config_->getItemValue("global", "newestversion");
      if (newestVersionRecorded.isEmpty()) {
        newestVersionRecorded = FQTERM_VERSION_STRING;
        config_->setItemValue("global", "newestversion", newestVersionRecorded);
      }
      QString newestVersionReleased = getNewestVersion();

      QString updatePrompt = config_->getItemValue("global", "updateprompt");
      if (updatePrompt == "0") {
        //if newest version > recorded version, still prompt.
        if (versionCompare(newestVersionRecorded, newestVersionReleased) > 0) {
          config_->setItemValue("global", "updateprompt", "1");
          promptUpdate();
        }
      }
      else {
        config_->setItemValue("global", "updateprompt", "1");
        //if newest version > version in use, prompt.
        if (versionCompare(FQTERM_VERSION_STRING, newestVersionReleased) > 0) {
          promptUpdate();
        }
      }
    }
  }
  deleteLater();
}

FQTermAutoUpdater::FQTermAutoUpdater(QObject* parent, FQTermConfig* config) 
: QObject(parent),
  config_(config),
  versionInfoFile_(0),
  checkDone_(false) {
  updateChecker_ = new QHttp(this);
}

FQTermAutoUpdater::~FQTermAutoUpdater()
{

}

} //FQTerm namespace

#include "fqterm_autoupdate.moc"
