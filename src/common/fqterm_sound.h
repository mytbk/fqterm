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
