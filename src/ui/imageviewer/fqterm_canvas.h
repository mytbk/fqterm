// SPDX-License-Identifier: GPL-2.0-or-later

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
