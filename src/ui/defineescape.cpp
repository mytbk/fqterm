// SPDX-License-Identifier: GPL-2.0-or-later

#include "defineescape.h"
#include "fqterm.h"

namespace FQTerm {

DefineEscapeDialog::~DefineEscapeDialog()
{
}

DefineEscapeDialog::DefineEscapeDialog(QString& strEsc, QWidget *parent_ /*= 0*/, Qt::WindowFlags fl /*= 0*/)
: QDialog(parent_, fl), strEsc_(strEsc)
{
  ui_.setupUi(this);
  ui_.edtEscape->setText(strEsc_);
  FQ_VERIFY(connect(ui_.btnOK, SIGNAL(clicked()), this, SLOT(onOK())));
  FQ_VERIFY(connect(ui_.btnCancel, SIGNAL(clicked()), this, SLOT(onCancel())));
}

void DefineEscapeDialog::onOK()
{
  strEsc_ = ui_.edtEscape->text();
  done(1);
}

void DefineEscapeDialog::onCancel()
{
  done(0);
}

    void DefineEscapeDialog::setTitleAndText(const QString &title, const QString &text)
    {
        ui_.lblEscape->setText(text);
        this->setWindowTitle(title);
    }

    void DefineEscapeDialog::setEditText(const QString &text)
    {
        ui_.edtEscape->setText(text);
    }
        
} //namespace FQTerm

#include "defineescape.moc"
