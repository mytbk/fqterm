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

#include <QCloseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QMatrix>
#include <QToolBar>
#include <QActionGroup>
#include <QToolButton>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <QImageReader>
#include <QMovie>

#include "fqterm_canvas.h"
#include "fqterm.h"
#include "fqterm_config.h"
#include "fqterm_path.h"
#include "fqterm_filedialog.h"

namespace FQTerm {

FQTermCanvas::FQTermCanvas(FQTermConfig * config, QWidget *parent, Qt::WindowFlags f)
    : QScrollArea(parent),
      adjustMode_(Fit),
      aspectRatioMode_(Qt::KeepAspectRatio){
  // TODO: dirty trick

  if (f == 0) {
    isEmbedded = true;
  } else {
    isEmbedded = false;
  }

  config_ = config; 

  menu_ = new QMenu(parent);
  menu_->addAction(tr("zoom 1:1"), this, SLOT(oriSize()), tr("Ctrl+Z"));
  menu_->addAction(tr("adjust size"), this, SLOT(autoAdjust()), tr("Ctrl+X"));
  menu_->addSeparator();
  QAction* zoomInAction = menu_->addAction(tr("zoom in"), this,
                                           SLOT(zoomIn()), tr("Ctrl+="));
  QAction* zoomOutAction = menu_->addAction(tr("zoom out"), this,
                                            SLOT(zoomOut()), tr("Ctrl+-"));
  if (!isEmbedded) {
    menu_->addAction(tr("fullscreen"), this, SLOT(fullScreen()), tr("Ctrl+F"));
  }
  menu_->addSeparator();
  menu_->addAction(tr("rotate CW 90"), this, SLOT(cwRotate()),
                   tr("Ctrl+]"));
  menu_->addAction(tr("rotate CCW 90"), this, SLOT(ccwRotate()),
                   tr("Ctrl+["));
  menu_->addSeparator();
  gifPlayAction_ = menu_->addAction(tr("play gif"), this,
    SLOT(playGIF()), tr("Ctrl+/"));
  menu_->addSeparator();
  menu_->addAction(tr("save as"), this, SLOT(saveImage()), tr("Ctrl+S"));
  menu_->addAction(tr("copy to"), this, SLOT(copyImage()), tr("Ctrl+C"));
  menu_->addAction(tr("silent copy"), this, SLOT(silentCopy()),
    tr("Shift+S"));

  if (!isEmbedded) {
    menu_->addAction(tr("delete"), this, SLOT(deleteImage()), tr("Ctrl+D"));

    menu_->addSeparator();
    menu_->addAction(tr("exit"), this, SLOT(close()), tr("Ctrl+Q"));
  }

  toolBar_ = new QToolBar(this);

  toolBar_->addAction(QIcon(getPath(RESOURCE) + "pic/ViewerButtons/open.png"),
                      tr("Open Dir"), this, SLOT(openDir()));

  zoomInAction->setIcon(
      QIcon(getPath(RESOURCE) + "pic/ViewerButtons/zoomin.png"));
  zoomOutAction->setIcon(
      QIcon(getPath(RESOURCE) + "pic/ViewerButtons/zoomout.png"));

  toolBar_->addAction(zoomInAction);
  toolBar_->addAction(zoomOutAction);

  QActionGroup* gifGroup = new QActionGroup(this);
  gifPlayAction_->setIcon(
    QIcon(getPath(RESOURCE) + "pic/ViewerButtons/play_gif.png"));
  toolBar_->addAction(gifPlayAction_);

  QActionGroup* showSettingGroup = new QActionGroup(this);
  QMenu* showSettingMenu = new QMenu(this);

  QAction* showSettingFitAction =
      showSettingMenu->addAction(tr("show Fit"), this, SLOT(SetAdjustMode()));
  showSettingFitAction->setCheckable(true);
  showSettingFitAction->setData(Fit);
  showSettingGroup->addAction(showSettingFitAction);

  QAction* showSettingMaxFitAction =
      showSettingMenu->addAction(tr("show MaxFit"),
                                 this, SLOT(SetAdjustMode()));
  showSettingMaxFitAction->setCheckable(true);
  showSettingMaxFitAction->setData(MaxFit);
  showSettingGroup->addAction(showSettingMaxFitAction);

  QAction* showSettingOriginAction =
      showSettingMenu->addAction(tr("show Origin"),
                                 this, SLOT(SetAdjustMode()));
  showSettingOriginAction->setCheckable(true);
  showSettingOriginAction->setData(Origin);
  showSettingGroup->addAction(showSettingOriginAction);

  QAction* showSettingStrechAction =
      showSettingMenu->addAction(tr("show Stretch"),
                                 this, SLOT(SetAdjustMode()));
  showSettingStrechAction->setCheckable(true);
  showSettingStrechAction->setData(Stretch);
  showSettingGroup->addAction(showSettingStrechAction);

  QToolButton* showSettingButton = new QToolButton(this);
  QAction* dummyAction = new QAction(
      QIcon(getPath(RESOURCE) + "pic/ViewerButtons/adjustsize.png"),
      tr("Adjust Mode"), showSettingButton);
  showSettingButton->setDefaultAction(dummyAction);
  showSettingButton->setMenu(showSettingMenu);
  showSettingButton->setPopupMode(QToolButton::InstantPopup);

  showSettingFitAction->setChecked(true);

  toolBar_->addWidget(showSettingButton);



#if QT_VERSION >= 0x040200
  setAlignment(Qt::AlignCenter);
#endif
  label_ = new QLabel(viewport());
  label_->setScaledContents(true);
  label_->setAlignment(Qt::AlignCenter);
  label_->setText(tr("No Preview Available"));
  setWidget(label_);
  //  resize(200, 100);
}

FQTermCanvas::~FQTermCanvas() {
  //delete label;
  //delete m_pMenu;
}

void FQTermCanvas::oriSize() {
  useAdjustMode_ = false;
  imageSize_ = image_.size();
  adjustSize(size());
}

void FQTermCanvas::zoomIn() {
  useAdjustMode_ = false;
  resizeImage(0.05f);
}

void FQTermCanvas::zoomOut() {
  useAdjustMode_ = false;
  resizeImage(-0.05f);
}

void FQTermCanvas::autoAdjust() {
  useAdjustMode_ = true;
  adjustSize(size());
}

void FQTermCanvas::cwRotate() {
  rotateImage(90);
}

void FQTermCanvas::ccwRotate() {
  rotateImage(-90);
}

void FQTermCanvas::fullScreen() {
  if (!isFullScreen()) {
    showFullScreen();
  } else {
    showNormal();
  }
}

void FQTermCanvas::loadImage(const QString& name, bool performAdjust) {
  if (label_->movie()) {
    label_->movie()->stop();
    label_->setMovie(NULL);
  }

  gifPlayAction_->setEnabled(name.endsWith("gif"));


  bool res = image_.load(name);
  if (!res) {
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    for (QList<QByteArray>::iterator it = formats.begin();
         !res && it != formats.end();
         ++it) {
      res = image_.load(name, it->data());
    }
  }
  if (!image_.isNull()) {
    fileName_ = QFileInfo(name).absoluteFilePath();
    setWindowTitle(QFileInfo(name).fileName());

    useAdjustMode_ = true;

    if (!isEmbedded) {
      QSize szView(image_.size());
      szView.scale(640, 480, aspectRatioMode_);

      if (szView.width() < image_.width()) {
        imageSize_ = szView;
        label_->resize(imageSize_);
        label_->clear();
        label_->setAlignment(Qt::AlignCenter);
        label_->setPixmap(scaleImage(imageSize_));
        
        if (!isEmbedded) {
          resize(szView *1.1);
        }
      } else {
        imageSize_ = image_.size();
        label_->resize(imageSize_);
        label_->setPixmap(QPixmap::fromImage(image_));
        if (!isEmbedded) {
          resize(imageSize_ + QSize(5, 5));
        }
      }
    }
    if (isEmbedded) {
      label_->hide();
      if (performAdjust) autoAdjust();
      else label_->setPixmap(QPixmap::fromImage(image_));
      
      label_->show();
    }

  } else {
    FQ_TRACE("canvas", 1) << "Can't load the image: " << name;
  }
}

void FQTermCanvas::playGIF() {
  if (fileName_.endsWith("gif")) {
    gifPlayer_.setFileName(fileName_);
    if (gifPlayer_.isValid()) {
      label_->setMovie(&gifPlayer_);
      gifPlayer_.stop();
      gifPlayer_.start();
      return;
    }
  }
}

void FQTermCanvas::resizeImage(float ratio) {
  QSize szImg = imageSize_;
  szImg *= (1+ratio);
  //we dont need so big
  if (szImg.width() > 10000 || szImg.height() > 10000 ||
      szImg.width() < 1 || szImg.height() < 1) {
    return ;
  }
  imageSize_ = szImg;

  if (!isFullScreen() && !isEmbedded) {
    resize(imageSize_ *1.1);
  } else {
    adjustSize(size());
  }
}

void FQTermCanvas::rotateImage(float ang) {
  QMatrix wm;

  wm.rotate(ang);

  image_ = image_.transformed(wm, Qt::SmoothTransformation);

  imageSize_ = image_.size();

  adjustSize(size());
}

void FQTermCanvas::copyImage() {
  QFileInfo fi(fileName_);
//  QString strSave =
//      QFileDialog::getSaveFileName(
//          this,tr("Choose a filename to save under"),
//          QDir::currentPath() + fi.fileName());
  FQTermFileDialog fileDialog(config_);
  QString strSave = fileDialog.getSaveName(fi.fileName(), "");
  if (strSave.isEmpty()) {
    return ;
  }
  QFile file(fileName_);
  if (file.open(QIODevice::ReadOnly)) {
    QFile save(strSave);
    if (save.open(QIODevice::WriteOnly)) {
      QByteArray ba = file.readAll();
      QDataStream ds(&save);
      ds.writeRawData(ba, ba.size());
      save.close();
    }
    file.close();
  }
}

void FQTermCanvas::silentCopy() {
  // save it to $savefiledialog
  QString strPath = config_->getItemValue("global", "savefiledialog");

  QFileInfo fi(fileName_);
  QString strSave = strPath + "/" + fi.fileName();

  fi.setFile(strSave);

  // add (%d) if exist
  int i = 1;
  while (fi.exists()) {
    strSave = QString("%1/%2(%3).%4").arg(fi.dir().path()).arg
              (fi.completeBaseName()).arg(i).arg(fi.suffix());
    fi.setFile(strSave);
  }

  // copy it
  QFile file(fileName_);
  if (file.open(QIODevice::ReadOnly)) {
    QFile save(strSave);
    if (save.open(QIODevice::WriteOnly)) {
      QByteArray ba = file.readAll();
      QDataStream ds(&save);
      ds.writeRawData(ba, ba.size());
      save.close();
    }
    file.close();
  }
}

QPixmap FQTermCanvas::scaleImage(const QSize &sz) {
  return QPixmap::fromImage(
      image_.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void FQTermCanvas::moveImage(float dx, float dy) {
  scrollContentsBy(int(widget()->width()*dx), int(widget()->height() *dy));
}

void FQTermCanvas::saveImage() {

  QFileInfo fi(fileName_);
  FQTermFileDialog fileDialog(config_);
  QString strSave = fileDialog.getSaveName(fi.fileName(), "");
  if (strSave.isEmpty()) {
    return ;
  }
  QString fmt = fi.suffix().toUpper();
  if (!image_.save(strSave, fmt.toLatin1())) {
    QMessageBox::warning(this, tr("Failed to save file"),
                         tr("Cant save file, maybe format not supported"));
  }
}

void FQTermCanvas::deleteImage() {
  QFile::remove(fileName_);
  close();
}

void FQTermCanvas::closeEvent(QCloseEvent *ce) {
  if (!isEmbedded) {
    delete this;
  }
}

void FQTermCanvas::viewportResizeEvent(QResizeEvent *re) {
  adjustSize(re->size());
}

void FQTermCanvas::mousePressEvent(QMouseEvent *me) {
  /* remove this to avoid click by mistake
     if(me->button()&LeftButton)
     {
     close();
     return;
     }
  */
  if (me->button() &Qt::RightButton) {
    menu_->popup(me->globalPos());
  }
}

void FQTermCanvas::keyPressEvent(QKeyEvent *ke) {
  switch (ke->key()) {
    case Qt::Key_Escape:
      if (!isEmbedded) {
        if (isFullScreen()) {
          showNormal();
        } else {
          close();
        }
      }

      break;
    case Qt::Key_D:
      if (!isEmbedded){
        deleteImage();
      }
      break;
    case Qt::Key_Q:
      if (!isEmbedded){
        close();
      }
      break;
  }
}

void FQTermCanvas::adjustSize(QSize szView) {
  szView -= QSize(2 * frameWidth(), 2 * frameWidth());
  if (label_->pixmap() == NULL && image_.isNull()) {
    label_->resize(szView);
    return ;
  }

  QSize szImg = imageSize_;

  if (useAdjustMode_) {
    szImg = image_.size();
    switch(adjustMode_)
    {
    case MaxFit:
      szImg.scale(szView, aspectRatioMode_);
      break;
    case Fit:
      if (szImg.width() > szView.width() || szImg.height() > szView.height() ||
        szImg.width() < image_.width()) {
          szImg.scale(szView, aspectRatioMode_);
      }
      szImg = szImg.boundedTo(image_.size());
      if (szImg.width() < 1 || szImg.height() < 1) {
        return;
      }
      break;
    case Origin:
      break;
    case Stretch:
      szImg = szView;
      break;
    }
  }

  imageSize_ = szImg;
  label_->resize(imageSize_);
  label_->setPixmap(scaleImage(imageSize_));
}

void FQTermCanvas::resizeEvent(QResizeEvent * event) {
  if (useAdjustMode_){
    autoAdjust();
  }
  else adjustSize(size());
  QScrollArea::resizeEvent(event);
}

QMenu* FQTermCanvas::menu() {
  return menu_;
}


QToolBar* FQTermCanvas::ToolBar() {
  return toolBar_;
}

void FQTermCanvas::SetAdjustMode(AdjustMode am) {
  adjustMode_ = am;
  if (adjustMode_ == Stretch) {
    aspectRatioMode_ = Qt::IgnoreAspectRatio;
  }
  else {
    aspectRatioMode_ = Qt::KeepAspectRatio;
  }
  autoAdjust();
}

void FQTermCanvas::SetAdjustMode() {
  SetAdjustMode(AdjustMode((((QAction*)sender())->data()).toInt()));
}

void FQTermCanvas::openDir() {
  QString poolPath = config_->getItemValue("preference", "pool");
  if (poolPath.isEmpty()) {
    poolPath = getPath(USER_CONFIG) + "pool/";
  }
#ifdef WIN32
  QString path = "file:///" + poolPath;
#else
  QString path = "file://" + poolPath;
#endif
  QDesktopServices::openUrl(path);
}

void FQTermCanvas::updateImage(const QString& filename)
{
  if (QFileInfo(filename).absoluteFilePath().toLower() == fileName_) {
    loadImage(fileName_);
  }
  
}


}  // namespace FQTerm

#include "fqterm_canvas.moc"
