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

#include "shortcutdialog.h"
#include "fqterm_shortcuthelper.h"

#include <QTableWidget>
#include <QKeyEvent>
#include <QGridLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QStringList>
#include <QScrollBar>
#include <QLabel>
#include <QSignalMapper>
namespace FQTerm
{
FQTermShortcutDialog::FQTermShortcutDialog(FQTermShortcutHelper* helper, QWidget *parent_, Qt::WindowFlags fl)
  : QDialog(parent_, fl),
    helper_(helper),
    table_(NULL) {
  setWindowTitle(tr("Shortcut Setting"));
  //grabKeyboard();
  if (helper_)
  {
    QSignalMapper* signalMapper = new QSignalMapper(this);
    int row = FQTermShortcutHelper::FQTERM_APPLICATION_SHORTCUT_END - FQTermShortcutHelper::FQTERM_APPLICATION_SHORTCUT_START - 1;
    int column = 3;
    table_ = new QTableWidget(row, column, this);
    table_->clear();
    QStringList header;
    header << tr("Description") << tr("Shortcut") << "";
    table_->setHorizontalHeaderLabels(header);
    for (int i = 0; i < row; ++i) {
      for (int j = 0; j < column; ++j) {
        if (j == 0) {
          QTableWidgetItem* item = new QTableWidgetItem;
          table_->setItem(i, j, item);
          item->setFlags(item->flags() & ~Qt::ItemIsEditable);
          item->setText(helper_->getShortcutDescription(i + 1));
        } else if (j == 1) {
          FQTermShortcutTableWidget* stw = new FQTermShortcutTableWidget(table_);
          stw->setReadOnly(true);
          stw->setText(helper_->getShortcutText(i + 1));
          table_->setCellWidget(i, j, stw);
        }else {
          QPushButton* btn = new QPushButton(tr("default"), table_);
          table_->setCellWidget(i, j, btn);
          FQ_VERIFY(connect(btn, SIGNAL(clicked()), signalMapper, SLOT(map())));
         signalMapper->setMapping(btn, i);
        }
      }
    }
    table_->resizeColumnsToContents();
    int tableWidth = table_->horizontalHeader()->length();
    if (table_->horizontalScrollBar() && table_->horizontalScrollBar()->isVisible()) {
      tableWidth += table_->horizontalScrollBar()->width();
    }
    QPushButton* okBtn = new QPushButton(tr("OK"), this);
    FQ_VERIFY(connect(okBtn, SIGNAL(clicked()), this, SLOT(okBtnClicked())));
    QPushButton* applyBtn = new QPushButton(tr("Apply"), this);
    FQ_VERIFY(connect(applyBtn, SIGNAL(clicked()), this, SLOT(applyBtnClicked())));
    QPushButton* cancelBtn = new QPushButton(tr("Cancel"), this);
    FQ_VERIFY(connect(cancelBtn, SIGNAL(clicked()), this, SLOT(cancelBtnClicked())));
    QPushButton* resetBtn = new QPushButton(tr("Reset All"), this);
    FQ_VERIFY(connect(resetBtn, SIGNAL(clicked()), this, SLOT(resetBtnClicked())));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    QGridLayout* layout = new QGridLayout(this);
    QLabel* label = new QLabel(tr("Press key/combines on Shortcut column.\nNote Del is reserved for clear shortcut setting."));
    layout->addWidget(label, 0, 0, 1, -1);
    layout->addWidget(table_, 1, 0, 1, -1);
    layout->addWidget(okBtn, 2, 0);
    layout->addWidget(applyBtn, 2, 1);
    layout->addWidget(cancelBtn, 2, 2);
    layout->addWidget(resetBtn, 2, 3);
    setLayout(layout);
    int left = 0;
    int right = 0;
    layout->getContentsMargins(&left, NULL, &right, NULL);
    table_->horizontalHeader()->setStretchLastSection(true);
    tableWidth *= 1.1;
    resize(tableWidth + left + right, height());
    FQ_VERIFY(connect(signalMapper, SIGNAL(mapped(int)),this, SLOT(defaultClicked(int))));
  }
}

FQTermShortcutDialog::~FQTermShortcutDialog() {
  //releaseKeyboard();
}

void FQTermShortcutDialog::defaultClicked(int row) {
  ((QLineEdit*)table_->cellWidget(row, 1))->setText(helper_->getShortcutDefaultText(row + 1));
}

void FQTermShortcutDialog::okBtnClicked() {
  applyChanges();
  done(true);
}

void FQTermShortcutDialog::applyBtnClicked() {
  applyChanges();
}

void FQTermShortcutDialog::cancelBtnClicked() {
  done(false);
}

void FQTermShortcutDialog::resetBtnClicked() {
  int row = FQTermShortcutHelper::FQTERM_APPLICATION_SHORTCUT_END - FQTermShortcutHelper::FQTERM_APPLICATION_SHORTCUT_START - 1;
  for (int i = 0; i < row; ++i) {
    ((QLineEdit*)table_->cellWidget(i, 1))->setText(helper_->getShortcutDefaultText(i + 1));
  }
}

void FQTermShortcutDialog::applyChanges() {
  int row = FQTermShortcutHelper::FQTERM_APPLICATION_SHORTCUT_END - FQTermShortcutHelper::FQTERM_APPLICATION_SHORTCUT_START - 1;
  for (int i = 0; i < row; ++i) {
    helper_->setShortcutText(i + 1, ((QLineEdit*)table_->cellWidget(i, 1))->text());
  }
}

void FQTermShortcutTableWidget::keyReleaseEvent(QKeyEvent * event) {
  if (event) {
    event->accept();
  }
}

void FQTermShortcutTableWidget::keyPressEvent(QKeyEvent * event) {
  if (event) {
    event->accept();
    if (event == QKeySequence::Delete) {
      setText("");
    } else {
      //Thank you hooey.
      int key = event->key();
      Qt::KeyboardModifiers mod = event->modifiers();

      if (key == Qt::Key_Shift  || 
          key == Qt::Key_Control || 
          key == Qt::Key_Meta || 
          key == Qt::Key_Alt || 
          key == Qt::Key_AltGr)
          return;
      QString text = "";
      if (mod != Qt::NoModifier) {
          QKeySequence seqMod(mod);
          text = seqMod.toString(QKeySequence::NativeText);
      }
      QKeySequence seqKey(key);
      text += seqKey.toString(QKeySequence::NativeText);
      setText(text);
    }
  }
}

}//namespace FQTerm


#include "shortcutdialog.moc"
