// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SITEMANGER_H
#define FQTERM_SITEMANGER_H

#include "addrdialog.h"

#include "ui_sitemanager.h"
#include "ui_quickdialog.h"

class QListWidgetItem;

namespace FQTerm {
class FQTermParam;
class FQTermConfig;

class siteDialog: public QDialog {
  Q_OBJECT;
 public:
  siteDialog(QWidget *parent_ = 0, Qt::WindowFlags fl = 0);;
  ~siteDialog();

  FQTermParam currentParameter();

  int currentSiteIndex();

 private:
  void connector();

  void resizeEvent(QResizeEvent * re);

  //Note: changes are made on config_
  void saveCurrentParam();
  void loadCurrentParam();

  void setParamFromUI();
  void setUIFromParam();

  void previewFont();

  void swapSite(int first, int second);
  int moveSite(int pos, int offset);
  void forcedSetCurrentSite(int row);
  void setCurrentSite(int row);

  QMessageBox::StandardButton checkModification(int row);

  void removeSite(int row);

  void close(int doneValue);
  void setting(addrDialog::Tabs tab);


 protected slots:
  void onSelectSite(QListWidgetItem* current, QListWidgetItem* previous);
  void siteNameChanged(QString name);
  void setSiteSelected();

  void onSelectProtocol(int index);

  void onReset();
  void onApply();
  void onUp();
  void onDown();
  void onNew();
  void onDelete();
  void onClose();
  void onConnect();

  void onAdvance();
  void onProxy();
  void onAutoLogin();
  void onDblClicked(QListWidgetItem * item);

 private:
  FQTermConfig *config_;
  FQTermParam param_;

  Ui::siteManager ui_;

  static int ports[]; //telnet, ssh1, ssh2
};

}  //FQTerm namespace

#endif  // FQTERM_SITEMANGER_H
