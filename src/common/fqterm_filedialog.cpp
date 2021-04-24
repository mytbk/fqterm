// SPDX-License-Identifier: GPL-2.0-or-later

#include "fqterm_filedialog.h"
#include "fqterm_trace.h"
#include "fqterm_path.h"
#include "fqterm_config.h"

/* 
 *  Constructs a articleDialog as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */

namespace FQTerm {

FQTermFileDialog::FQTermFileDialog(FQTermConfig *config, QWidget *parent, Qt::WindowFlags fl)
  : QFileDialog(parent, fl) {

  configDir = getPath(USER_CONFIG) + "/";
  userConfig = configDir + "fqterm.cfg";
  strSection = "previous";
  strSave = "save";
  strOpen = "open";
  config_ = config;
}

/*
 *  Destroys the object and frees any allocated resources
 */
FQTermFileDialog::~FQTermFileDialog() {
  // no need to delete child widgets, Qt does it all for us
}



QString FQTermFileDialog::getSaveName(const QString &fileToSave, const QString &hints, QWidget *widget) {
  QString saveFile;
  QString strPrevSave, saveName;
  QFileDialog fileDialog(widget);

  if (config_!=NULL && config_->load(userConfig)) {
    strPrevSave = config_->getItemValue(strSection, strSave);
  }

  QString strTmp(fileToSave);
#if !defined(Q_OS_WIN32) || !defined(_OS_WIN32_)
  if (strTmp.toLocal8Bit().contains("/")) {
    strTmp.replace(QString("/"), QString("_"));
  }
#endif

  if (strPrevSave.isEmpty()) {
    saveFile = configDir + strTmp;
  } else {
    saveFile = strPrevSave + "/" + strTmp;
  }

  QString realHints = (hints.isEmpty() ? "*" : hints);
  saveName = fileDialog.getSaveFileName(widget,
                                        tr("Save As..."),
                                        saveFile, realHints);

  QFileInfo fi(saveName);

  if (config_!=NULL && !saveName.isEmpty()) {
    config_->setItemValue(strSection, strSave, fi.absolutePath());
    config_->save(userConfig);
  }

  return saveName;
}

QString FQTermFileDialog::getOpenName(const QString &title, const QString &hints, QWidget *widget) {
  QString strPrevOpen;
  QString openName;
  QFileDialog fileDialog(widget);

  if (config_!=NULL){
      strPrevOpen = (config_->load(userConfig) ? config_->getItemValue(strSection, strOpen) : "./");
  }
  
  QString realHints = (hints.isEmpty() ? "*" : hints);
  QString realTitle = (title.isEmpty() ? "Choose a file to open" : title);
  openName = fileDialog.getOpenFileName(widget, realTitle, strPrevOpen, realHints);

  if (config_!=NULL && !openName.isEmpty()) {
    config_->setItemValue(strSection, strOpen, QFileInfo(openName).absolutePath());
    config_->save(userConfig);
  }

  return openName;
}

QStringList FQTermFileDialog::getOpenNames(const QString &title, const QString &hints, QWidget *widget) {
  QString openDir, strPrevOpen;
  QStringList openNames;
  QFileDialog fileDialog(widget);

  if (config_!=NULL){
      strPrevOpen = (config_->load(userConfig) ? config_->getItemValue(strSection, strOpen) : "./");
  }
  
  QString realHints = (hints.isEmpty() ? "*" : hints);
  QString realTitle = (title.isEmpty() ? "Choose files to open" : title);
  openNames = fileDialog.getOpenFileNames(widget, realTitle, strPrevOpen, realHints);

  if (config_!=NULL && !openNames.isEmpty() && !openNames.at(0).isEmpty()) {
    openDir = QFileInfo(openNames.at(0)).absolutePath();
    config_->setItemValue(strSection, strOpen, openDir);
    config_->save(userConfig);
  }

  return openNames;
}

QString FQTermFileDialog::getExistingDirectory(const QString &title, const QString &hints, QWidget *widget) {

  QString strPrevOpen;
  QFileDialog fileDialog(widget);

  if (config_!=NULL && hints.isEmpty()) {
    strPrevOpen = (config_->load(userConfig) ? config_->getItemValue(strSection, strOpen) : "./");
  } else {
    strPrevOpen = hints;
  }

  QString realTitle = (title.isEmpty() ? "Open a directory" : title);
  QString dir = fileDialog.getExistingDirectory(widget,
                                                realTitle,
                                                strPrevOpen,
                                                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (config_!=NULL && !dir.isEmpty()) {
    config_->setItemValue(strSection, strOpen, QFileInfo(dir).absolutePath());
    config_->save(userConfig);
  }

  return dir;
}

} // namespace FQTerm
