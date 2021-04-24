// SPDX-License-Identifier: GPL-2.0-or-later

#include "articledialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include "fqterm_trace.h"
#include "fqterm_path.h"
#include "fqterm_config.h"
#include "fqterm_filedialog.h"

/* 
 *  Constructs a articleDialog as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */

namespace FQTerm {

articleDialog::articleDialog(FQTermConfig *config, QWidget *parent, Qt::WindowFlags fl)
  : QDialog(parent, fl) {
  ui_.setupUi(this);
  connectSlots();

  config_ = config;
}

/*
 *  Destroys the object and frees any allocated resources
 */
articleDialog::~articleDialog() {
  // no need to delete child widgets, Qt does it all for us
}


void articleDialog::connectSlots() {
  FQ_VERIFY(connect(ui_.selectButton, SIGNAL(clicked()), this, SLOT(onSelect())));
  FQ_VERIFY(connect(ui_.copyButton, SIGNAL(clicked()), this, SLOT(onCopy())));
  FQ_VERIFY(connect(ui_.saveButton, SIGNAL(clicked()), this, SLOT(onSave())));
  FQ_VERIFY(connect(ui_.closeButton, SIGNAL(clicked()), this, SLOT(onClose())));
}

void articleDialog::onSelect() {
  ui_.textBrowser->selectAll();
}

void articleDialog::onCopy() {
  ui_.textBrowser->copy();
  // 	QString strText = textBrowser->selectedText();
  // 
  // 	QClipboard *clipboard = QApplication::clipboard();
  // 	
  // 	#if (QT_VERSION>=0x030100)
  // 		clipboard->setText(strText, QClipboard::Selection );
  // 		clipboard->setText(strText, QClipboard::Clipboard );
  // 	#else
  // 		clipboard->setText(strText);
  // 	#endif
}

void articleDialog::onSave() {

  QString filename;
  FQTermFileDialog fileDialog(config_);
  filename = fileDialog.getSaveName(NULL, "");

  if (!filename.isNull()) {
    QFile f(filename);
    if ((f.open(QIODevice::WriteOnly))) {
      f.write(articleText_.toLocal8Bit());
      f.close();
    } else {
      QMessageBox mb("Access file error:", filename, QMessageBox::Warning, 0, 0, 0,
                     this, 0);
      mb.exec();
    }
  }
}

void articleDialog::onClose() {
  done(0);
}

} // namespace FQTerm

#include "articledialog.moc"
