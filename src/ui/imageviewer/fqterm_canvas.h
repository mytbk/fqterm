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

#ifndef FQTERM_CANVAS_H
#define FQTERM_CANVAS_H

#include <QScrollArea>
#include <QImage>
#include <QMovie>

class QAction;
class QLabel;
class QMenu;
class QCloseEvent;
class QMouseEvent;
class QKeyEvent;
class QResizeEvent;
class QToolBar;
class QImage;

namespace FQTerm {

class FQTermConfig;
class ExifExtractor;
class FQTermFileDialog;

class FQTermCanvas: public QScrollArea {
  Q_OBJECT;
 public:
  FQTermCanvas(FQTermConfig *, QWidget *parent_ = NULL, Qt::WindowFlags f = Qt::Window);
  ~FQTermCanvas();

  enum AdjustMode{Origin, Fit, Stretch, MaxFit};

  void updateImage(const QString& filename);
  void loadImage(const QString&, bool = true);
  QMenu* menu();
  QToolBar* ToolBar();
 public slots:
  void oriSize();
  void zoomIn();
  void zoomOut();
  void autoAdjust();
  void fullScreen();
  void saveImage();
  void copyImage();
  void silentCopy();
  void cwRotate();
  void ccwRotate();
  void deleteImage();
  void SetAdjustMode(AdjustMode am);
  void openDir();
  void playGIF();
 protected slots:
  void SetAdjustMode();
 protected:
  void resizeEvent (QResizeEvent * event);
  void moveImage(float, float);
  void resizeImage(float);
  void rotateImage(float);

  void closeEvent(QCloseEvent*);
  void mousePressEvent(QMouseEvent*);
  void keyPressEvent(QKeyEvent *ke);
  void viewportResizeEvent(QResizeEvent *re);
  void adjustSize(QSize);
  QPixmap scaleImage(const QSize &);
 protected:
  QLabel *label_;
  bool useAdjustMode_;
  QSize imageSize_;
  QString fileName_;
  QImage image_;
  QMovie gifPlayer_;
  QMenu *menu_;
  QToolBar *toolBar_;
  FQTermConfig * config_;
  QAction* gifPlayAction_;

  AdjustMode adjustMode_;
  Qt::AspectRatioMode aspectRatioMode_;
  // TODO: Very dirty trick, I hate it
  bool isEmbedded;
};

}  // namespace FQTerm

#endif  // FQTERM_CANVAS_H
