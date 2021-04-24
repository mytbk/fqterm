// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SHORTCUT_DIALOG_H
#define FQTERM_SHORTCUT_DIALOG_H
#include <QDialog>
#include <QLineEdit>
class QTableWidget;
namespace FQTerm
{
class FQTermShortcutHelper;

class FQTermShortcutTableWidget : public QLineEdit
{
public:
  FQTermShortcutTableWidget(QWidget* parent) : QLineEdit(parent) {
  }
protected:
  void keyReleaseEvent(QKeyEvent * event);
  void keyPressEvent(QKeyEvent * event);
};

class FQTermShortcutDialog : public QDialog
{
  Q_OBJECT; 
public:
  FQTermShortcutDialog(FQTermShortcutHelper* helper, QWidget *parent_ = 0, Qt::WindowFlags fl = Qt::Dialog);
  ~FQTermShortcutDialog();
private:
  FQTermShortcutHelper* helper_;
  QTableWidget* table_;
protected slots:
  void defaultClicked(int row);
  void okBtnClicked();
  void applyBtnClicked();
  void cancelBtnClicked();
  void resetBtnClicked();
private:
  void applyChanges();
};
}//namespace FQTerm


#endif //FQTERM_SHORTCUT_DIALOG_H
