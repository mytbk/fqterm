// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef __FQTERM_DEFINEESCAPE__
#define __FQTERM_DEFINEESCAPE__

#include <QDialog>
#include "ui_defineescape.h"
namespace FQTerm {

class DefineEscapeDialog : public QDialog {
  Q_OBJECT;
public:
  DefineEscapeDialog(QString& strEsc, QWidget *parent_ = 0, Qt::WindowFlags fl = 0);
  ~DefineEscapeDialog();
  void setTitleAndText(const QString &title, const QString &text);
  void setEditText(const QString &text);
    
protected slots:
  void onOK();
  void onCancel();
private:
  QString& strEsc_;
  Ui::dlgDefineEscape ui_;
};

} //namespace FQTerm

#endif //__FQTERM_DEFINEESCAPE__
