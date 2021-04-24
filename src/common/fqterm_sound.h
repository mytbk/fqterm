// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SOUND_H
#define FQTERM_SOUND_H

#include <QObject>
#include <QString>
#include <QThread>

#include "fqterm.h"

namespace FQTerm {
// By using QThread, we avoid blocking the main app.
// FIXME: However, whichever play method we use (ext|int),
// there are some WAVE files we can't play...

class FQTermSound: public QThread {
 Q_OBJECT;

 public:
  FQTermSound(const QString &filename, QObject *parent = 0,
    const char *name = 0);

  ~FQTermSound();

 public slots:
  void deleteInstance();

 protected:
  QString soundFile_;
  virtual void play() = 0;
  void run();
};

class FQTermSystemSound: public FQTermSound {
 public:
  FQTermSystemSound(const QString &filename, QObject *parent = 0,
    const char *name = 0)
    : FQTermSound(filename, parent, name) {
  }

 protected:
  void play();
};

class FQTermExternalSound: public FQTermSound {
 public:
  FQTermExternalSound(const QString &playername, const QString &filename,
    QObject *parent = 0, const char *name = 0)
    : FQTermSound(filename, parent, name), playerName_(playername) {
  }

 protected:
  QString playerName_;
  void play();
  void setPlayer(const QString &playername);
};

} // namespace FQTerm

#endif  // FQTERM_SOUND_H
