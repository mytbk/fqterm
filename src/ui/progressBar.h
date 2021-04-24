// SPDX-License-Identifier: GPL-2.0-or-later

//WARNING this is not meant for use outside this unit!

#ifndef FQTERM_PROGRESSBAR_H
#define FQTERM_PROGRESSBAR_H

#include <QProgressBar>
#include <QLabel>

class QLabel;
class QPushButton;

namespace FQTerm {
/**
 * @class KDE::ProgressBar
 * @short ProgressBar class with some useful additions
 */
class ProgressBar: public QProgressBar {
  friend class StatusBar;

 public:
  /** @param text a 1-6 word description of the progress operation */
  ProgressBar &setDescription(const QString &text);

  /** @param text eg. Scanning, Reading. The state of the operation */
  ProgressBar &setStatus(const QString &text);

  /** set the recipient slot for the abort button */
  ProgressBar &setAbortSlot(QObject *receiver, const char *slot);

  void setDone();

  QString description()const {
    return description_;
  }

 protected:
  ProgressBar(QWidget *parent, QLabel *label);
  ~ProgressBar();

  virtual void hide();

  QLabel *label_;
  QString description_;
  bool isFinished_;

  QPushButton *abortButton_;
};

}  // namespace FQTerm

#endif  // FQTERM_PROGRESSBAR_H
