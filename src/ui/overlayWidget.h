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

#ifndef FQTERMOVERLAYWIDGET_H
#define FQTERMOVERLAYWIDGET_H

#include <QFrame>
#include <QResizeEvent>
#include <QEvent>
#include <QHBoxLayout>

namespace FQTerm {
class OverlayWidget: public QFrame {
 public:
  /**
   * The widget is parented to the toplevelwidget of alignWidget,
   * this could be an issue if that widget has an autoAdd Layout
   */
  OverlayWidget(QWidget *parent, QWidget *anchor);
  virtual void reposition();

 protected:
  virtual void resizeEvent(QResizeEvent*);
  virtual bool eventFilter(QObject *, QEvent*);
  virtual bool event(QEvent*);

 private:
  QWidget *anchor_;
  QWidget *parent_;
};

}  // namespace FQTerm

#endif  //  FQTERMOVERLAYWIDGET_H
