// SPDX-License-Identifier: GPL-2.0-or-later

#include <QPixmap>
#include <QFile>
#include <QTextBrowser>
#include <QTextStream>
#include <QtGlobal>

#include "fqterm.h"
#include "fqterm_path.h"
#include "aboutdialog.h"

namespace FQTerm {

/*
 *  Constructs a aboutDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
aboutDialog::aboutDialog(QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl) {
  ui_.setupUi(this);

  QFile file(getPath(RESOURCE) + "credits");
  if (file.open(QIODevice::ReadOnly)) {
    QTextStream stream(&file);
    QString line;
    while (!stream.atEnd()) {
      // line of text excluding '\n'
      line += stream.readLine() + "\n";
    }
    ui_.TextBrowser->setPlainText(line);
    file.close();
  }
  QString aboutText = "FQTerm "  + QString(FQTERM_VERSION_STRING)
#ifdef FQTERM_GIT_REVISION
	  + QString("\ngit revision ") + QString(FQTERM_GIT_REVISION)
#endif
	  + QString("\n Built with Qt") + QT_VERSION_STR + QString("\n Running with Qt ") + qVersion();
   ui_.TextLabel->setText(aboutText);
}

/*
 *  Destroys the object and frees any allocated resources
 */
aboutDialog::~aboutDialog() {
  // no need to delete child widgets, Qt does it all for us
}

}  // namespace FQTerm

#include "aboutdialog.moc"
