// SPDX-License-Identifier: GPL-2.0-or-later

#include <QAction>
#include <QBuffer>
#include <QCursor>
#include <QComboBox>
#include <QDateTime>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSizeGrip>
#include <QString>
#include <QTextCodec>
#include <QTimer>
#include <QToolTip>
#include <QTransform>
#include <QTreeView>
#include <QTextEdit>
#include <QHeaderView>
#include <QImageReader>

#include "fqterm_canvas.h"
#include "fqterm_config.h"
#include "fqterm_exif_extractor.h"
#include "fqterm_filedialog.h"
#include "fqterm_path.h"
#include "fqterm_trace.h"
#include "imageviewer.h"

namespace FQTerm {

  typedef enum {
	V_UNKNOWN = -1,
	V_ORIGINAL = 0,
	V_SHADOW,
	V_BESTFIT,
	V_ZOOMIN,
	V_ZOOMOUT,
	V_ROTLEFT,
	V_ROTRIGHT,
	V_FULLSCREEN
  } slideViewModes;

  typedef enum {
    SORT_UNSORTED = -1,
    SORT_LIKE,
    SORT_NEW,
    SORT_TRASH,
    SORT_RECOVER,
    SORT_TITLE
  } slideSortFlags;


  typedef enum {
    STAT_UNKNOWN = -1,
    STAT_LIKE,
    STAT_NEW,
    STAT_TRASH,
    STAT_RECOVER,
    STAT_TITLE,
    STAT_ALL
  } slideStatus;

  typedef enum {
    GEN_UNKNOWN = -1,
    GEN_CANCEL,
    GEN_NOCHANGE,
    GEN_RESHUFFLE,
    GEN_NOTAG,
    GEN_INTRASH,
    GEN_INTRASHES,
    GEN_TOSAVE,
    GEN_TOSAVES,
    GEN_TRASH,
    GEN_TRASHES,
    GEN_NOTRASH,
    GEN_RECOVER,
    GEN_RECOVERS,
    GEN_NORECOVER,
    GEN_SAVE,
    GEN_SAVES,
    GEN_DELETE,
    GEN_DELETES,
    GEN_SEARCHWRAP,
    GEN_SEARCHNOMATCH
  } generalStatus;

  static const char *fqloraTagStatus[] = {
    "Like",
    "New",
    "Trash",
    "Recover",
    "Title"
  };

  static const char *fqloraGeneralStatus[] = {
    "Canceled.",
    "No changes for ",
    "Reshuffled by ",
    "No tags by ",
    "item in the trash.",
    "items in the trash.",
    "item to save.",
    "items to save.",
    "image trashed.",
    "images trashed.",
    "Nothing to trash.",
    "image recovered.",
    "images recovered.",
    "Nothing to recover.",
    "image saved in ",
    "images saved in ",
    "image deleted.",
    "images deleted.",
    "Search wrapped.",
    "No matches."
  };

  static const int toolBarFixedHeight = 40;
  static const int statusBarFixedHeight = 18;
  static const QSize sortBoxFixedSize(110, 25);

  static const QString &iconPath(const QString &fileName) {

    static QString p = "";

    if (!fileName.isEmpty()) {
      p = getPath(RESOURCE) + ICON_SOURCE + fileName;
    }

    return p;
  }

  const QString &isPlural(const int num, const int status) {

    static QString m = "";

    if (num == 1) {
      return (m = fqloraGeneralStatus[status]);
    } else if (num > 1 && status > GEN_UNKNOWN && status < GEN_DELETES) {
      return (m = fqloraGeneralStatus[status + 1]);
    }

    return (m = fqloraGeneralStatus[GEN_UNKNOWN]);
  }

  // get user's desktop rectangle
  static const QRect &desktopSize() {

	static QRect size;
    QDesktopWidget myDesktop;
	return ((size = myDesktop.screenGeometry(myDesktop.primaryScreen())));
  }

  // FQTermImageFlow
  FQTermImageFlow::~FQTermImageFlow() {

	delete statusBar_;
    delete imageMenu_;
	delete imageFlow_;

    statusBar_ = NULL;
    imageMenu_ = NULL;
    imageFlow_ = NULL;
  }
  
  FQTermImageFlow::FQTermImageFlow(FQTermConfig * config, QWidget *parent,
								   Qt::WindowFlags wflag) :
	FQTermImage(parent, wflag),
	imageFlow_(new ImageFlow(this)),
    imageMenu_(new ImageMenu(this)),
    statusBar_(new QStatusBar(this)),
    config_(config) {

    setWindowTitle(IMAGE_BROWSER_NAME);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Midlight);
	setFixedSize(desktopSize().width() / 1.3, desktopSize().height() / 1.9);

    FQ_VERIFY(connect(this, SIGNAL(isTrashEmpty()), this, SLOT(checkTrashState())));

    // emblem state
    FQ_VERIFY(connect(imageFlow_, SIGNAL(emblemStatus(const int)), imageMenu_, SLOT(updateEmblems(const int))));
    FQ_VERIFY(connect(imageMenu_, SIGNAL(toggleFlowStatus(const int)), imageFlow_, SLOT(toggleStatus(const int))));

    // clear state
    FQ_VERIFY(connect(imageFlow_, SIGNAL(clearStatus(const bool)), imageMenu_, SLOT(updateClear(const bool))));
    FQ_VERIFY(connect(imageMenu_, SIGNAL(clearImages()), this, SLOT(clearImages())));

    // save state
    FQ_VERIFY(connect(imageFlow_, SIGNAL(saveStatus(const bool)), imageMenu_, SLOT(updateSave(const bool))));
    FQ_VERIFY(connect(imageMenu_, SIGNAL(saveImages()), this, SLOT(saveImages())));

    // trash state
    FQ_VERIFY(connect(this, SIGNAL(trashStatus(const bool)), imageMenu_, SLOT(updateDustbin(const bool))));
    FQ_VERIFY(connect(imageMenu_, SIGNAL(recoverImages()), this, SLOT(recoverImages())));

	// constructs a tool bar and its tool buttons
	QAction *closeBrowserAct = new QAction(QIcon(iconPath("window-close.png")), tr("Close"), this);
	closeBrowserAct->setShortcut(QKeySequence(Qt::Key_Q));
	FQ_VERIFY(connect(closeBrowserAct, SIGNAL(triggered()), this, SLOT(close())));

	QAction *trashAllAct = new QAction(QIcon(iconPath("trash-empty.png")), tr("Trash All"), this);
	trashAllAct->setShortcut(QKeySequence(Qt::Key_Delete));
	FQ_VERIFY(connect(trashAllAct, SIGNAL(triggered()), this, SLOT(trashAllImages())));

    QLabel *sortLabel = new QLabel(this);
    sortLabel->setMargin(5);
    sortLabel->setToolTip("Reshuffle images by tags");
    sortLabel->setPixmap(QPixmap(iconPath("edit-shuffle.png")));

    QComboBox *sortBox = new QComboBox(this);
    sortBox->setFocusPolicy(Qt::ClickFocus);
    sortBox->setFont(QFont("Serif", 12));
    sortBox->setFrame(false);
    sortBox->setFixedSize(sortBoxFixedSize);
    sortBox->insertItem(0, QIcon(iconPath("emblem-like-16x16.png")), tr("Like "));
    sortBox->insertItem(1, QIcon(iconPath("emblem-new-16x16.png")), tr("New "));
    sortBox->insertItem(2, QIcon(iconPath("emblem-trash-16x16.png")), tr("Trash "));
    sortBox->insertItem(3, QIcon(iconPath("emblem-recover-16x16.png")), tr("Recover "));
    sortBox->insertItem(4, QIcon(iconPath("emblem-title-16x16.png")), tr("Title "));
    FQ_VERIFY(connect(sortBox, SIGNAL(currentIndexChanged(int)), this, SLOT(reshuffleImages(int))));

	QToolBar *toolBar = new QToolBar(this);
	toolBar->setFixedHeight(toolBarFixedHeight);
	toolBar->setIconSize(QSize(32, 32));
	toolBar->addAction(closeBrowserAct);
    toolBar->addSeparator();
    toolBar->addWidget(sortLabel);
    toolBar->addWidget(sortBox);
    toolBar->addSeparator();
	toolBar->addAction(trashAllAct);

	// constructs a status bar to show instant messages
	statusBar_->setSizeGripEnabled(false);
	statusBar_->setFixedHeight(statusBarFixedHeight);
	statusBar_->setFont(QFont("Serif", 10, QFont::Bold));
    FQ_VERIFY(connect(imageFlow_, SIGNAL(statusMessage(const QString &)), this, SLOT(showStatusMessage(const QString &))));
    FQ_VERIFY(connect(this, SIGNAL(statusMessage(const QString &)), this, SLOT(showStatusMessage(const QString &))));
	// constructs a horizontal layout
	QVBoxLayout *vBox = new QVBoxLayout(this);
	vBox->setMargin(0);
    vBox->setSpacing(0);
    vBox->addWidget(toolBar);
	vBox->addWidget(imageFlow_);
    vBox->addWidget(imageMenu_);
	vBox->addWidget(statusBar_);
	vBox->setEnabled(true);
	setLayout(vBox);
  }

  const QString &FQTermImageFlow::poolSource(void) const {

    static QString p = "";

    p = config_->getItemValue("preference", "pool");

    if (p.isEmpty()) {
      p = getPath(USER_CONFIG) + POOL_SOURCE;
    }

    return p;
  }

  const QString &FQTermImageFlow::trashSource(void) const {

    static QString p = "";

    p = poolSource() + TRASH_SOURCE;
    return p;
  }

  // these three functions below are kept for
  // API compatibility.
  void FQTermImageFlow::adjustItemSize() {

  }

  void FQTermImageFlow::scrollTo(const QString &filePath) {

  }

  void FQTermImageFlow::updateImage(const QString &filePath) {

  }

  // slots and functions
  void FQTermImageFlow::loadImages(const int status) {

    int i;
    int itemStatus = (status == STAT_RECOVER) ? status : STAT_UNKNOWN;
    QString comment;
    QFileInfoList sourceList = sortedList(poolSource());
    QFileInfoList targetList = imageFlow_->digest(STAT_ALL);

    for (i = 0; i < sourceList.count(); i++) {

      if (!targetList.contains(sourceList[i])) {

        QImage image;

        if (!image.load(sourceList[i].absoluteFilePath())) {
          QFileIconProvider iconProvider;
          image = iconProvider.icon(sourceList[i]).pixmap(desktopSize().height() / 6.0).toImage();
        } else {
          image = image.scaled(desktopSize().width() / 6.0, desktopSize().height() / 6.0,
            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }

        /* TODO */
        /* there should be a more flexible */
        /* method to deal with items' status */
        if (status == STAT_ALL) {
          itemStatus = STAT_NEW;
        }

        ImageFlowItem newItem(image, sourceList[i], comment,itemStatus);
        imageFlow_->add(newItem);
      }
    }
  }

  void FQTermImageFlow::reshuffleImages(const int status) {

    // there are images in the pool directory
    QList<qint64> itemKeys = imageFlow_->sort(status);

    if (imageFlow_->reorder(itemKeys)) {
      imageFlow_->setCurrentSlide(0);
      emit statusMessage(tr(fqloraGeneralStatus[GEN_RESHUFFLE]) + tr(fqloraTagStatus[status]));
    } else {
      emit statusMessage(tr(fqloraGeneralStatus[GEN_NOCHANGE]) + tr(fqloraTagStatus[status]));
    }
  }

  void FQTermImageFlow::saveImages() {

    QFileInfoList toSave = imageFlow_->digest(STAT_LIKE);

    if (!toSave.isEmpty()) {

      QMessageBox box;
      box.setWindowModality(Qt::NonModal);
      box.setWindowTitle("Information");
      box.setIconPixmap(QPixmap(iconPath("emblem-info.png")));
      box.setTextFormat(Qt::RichText);
      box.setText(QString("<b>%1</b> ").arg(toSave.count()) + isPlural(toSave.count(), GEN_TOSAVE));
      QPushButton *cancelButton = box.addButton(QMessageBox::Cancel);
      QPushButton *saveAllButton = box.addButton(QMessageBox::SaveAll);
      box.setDefaultButton(cancelButton);
      box.exec();

      if (box.clickedButton() == saveAllButton) {

        FQTermFileDialog fileDialog(config_);
        QString savePath =
          fileDialog.getExistingDirectory(tr("Save your likes under"), "*");

        if (savePath.isEmpty()) {
          emit statusMessage(tr(fqloraGeneralStatus[GEN_CANCEL]));
          return;
        }

        int i;
        for (i = 0; i < toSave.count(); i++) {
          QFile::rename(toSave[i].absoluteFilePath(), savePath + toSave[i].fileName());
        }

        imageFlow_->strip(STAT_LIKE);
        emit statusMessage(QString("%1 ").arg(i) + isPlural(i, GEN_SAVE) + savePath);
      } else {
        emit statusMessage(tr(fqloraGeneralStatus[GEN_CANCEL]));
      }
    } else {
      emit statusMessage(tr(fqloraGeneralStatus[GEN_NOTAG]) + tr(fqloraTagStatus[STAT_LIKE]));
    }
  }

  void FQTermImageFlow::checkTrashState() {

    QDir trashPath(trashSource());

    if (trashPath.count() <= 2) {
      emit trashStatus(false);
    } else {
      emit trashStatus(true);
    }
  }

  void FQTermImageFlow::clearImages() {

    int i;
    QString trashPath = trashSource();
    QDir trashDir(trashPath);
    QFileInfoList toRemove = imageFlow_->digest(STAT_TRASH);

    if (!trashDir.exists()) {
      trashDir.mkdir(trashPath);
    }

    if (!toRemove.isEmpty()) {

      for (i = 0; i < toRemove.count(); i++) {
        if (QFile::exists(toRemove[i].absoluteFilePath())) {
          QFile::rename(toRemove[i].absoluteFilePath(), trashPath + toRemove[i].fileName());
        }
      }

      imageFlow_->strip(STAT_TRASH);
      emit statusMessage(QString("%1 ").arg(i) + isPlural(i, GEN_TRASH));
      emit isTrashEmpty();
    } else {
      emit statusMessage(tr(fqloraGeneralStatus[GEN_NOTAG]) + tr(fqloraTagStatus[STAT_TRASH]));
    }
  }

  void FQTermImageFlow::trashAllImages() {

	int i;
	QString trashPath = trashSource();
	QDir trashDir(trashPath);
	QFileInfoList toRemove = imageFlow_->digest(STAT_ALL);

	if (!trashDir.exists()) {
	  trashDir.mkdir(trashPath);
	}

	if (imageFlow_->count() > 0) {
	  for (i = 0; i < imageFlow_->count(); i++) {
		if (QFile::exists(toRemove[i].absoluteFilePath())) {
		  QFile::rename(toRemove[i].absoluteFilePath(), trashPath + toRemove[i].fileName());
		}
	  }

	  imageFlow_->clear();
	  emit statusMessage(QString("%1 ").arg(i) + isPlural(i, GEN_TRASH));
	  emit isTrashEmpty();
	} else {
	  emit statusMessage(tr(fqloraGeneralStatus[GEN_NOTAG]) + tr(fqloraTagStatus[STAT_TRASH]));
	}
  }

  void FQTermImageFlow::recoverImages() {

    QString trashPath = trashSource();
    QDir trashDir(trashPath);

    if (!trashDir.exists()) {
      trashDir.mkdir(trashPath);
    } else if (trashDir.count() > 2) {

      int action = -1;;
      QFileInfoList toRecover = sortedList(trashPath);

      if (!toRecover.isEmpty()) {

        QMessageBox box;
        box.setWindowModality(Qt::NonModal);
        box.setWindowTitle("Take action");
        box.setIconPixmap(QPixmap(iconPath("emblem-info.png")));
        box.setTextFormat(Qt::RichText);
        box.setText(QString("<b>%1</b> ").arg(toRecover.count()) + isPlural(toRecover.count(), GEN_INTRASH));
        QPushButton *cancelButton = box.addButton(QMessageBox::Cancel);
        QPushButton *recoverButton = box.addButton(tr("Recover"), QMessageBox::ApplyRole);
        recoverButton->setIcon(QIcon(iconPath("button-recover.png")));
        QPushButton *deleteButton = box.addButton(tr("Delete"), QMessageBox::ApplyRole);
        deleteButton->setIcon(QIcon(iconPath("button-delete.png")));
        box.setDefaultButton(cancelButton);
        box.exec();

        if (box.clickedButton() == cancelButton) {
          emit statusMessage(tr(fqloraGeneralStatus[GEN_CANCEL]));
          return;
        } else if (box.clickedButton() == recoverButton) {
          action = 1;
        } else if (box.clickedButton() == deleteButton) {
          action = 0;
        }

        int i;
        QFile imageFile;
        QString poolPath = poolSource();

        for (i = 0; i < toRecover.count(); i++) {
          if (action == 1) {
            QFile::rename(toRecover[i].absoluteFilePath(), poolPath + toRecover[i].fileName());
          } else if (action == 0) {
            imageFile.setFileName(toRecover[i].absoluteFilePath());
            imageFile.remove();
          }
        }

        if (action == 1) {
          loadImages(STAT_RECOVER);
          emit statusMessage(QString("%1 ").arg(i) + isPlural(i, GEN_RECOVER));
        } else if (action == 0) {
          emit statusMessage(QString("%1 ").arg(i) + isPlural(i, GEN_DELETE));
        }
      }
    } else {

      emit statusMessage(tr(fqloraGeneralStatus[GEN_NORECOVER]));
    }

    emit isTrashEmpty();
  }


  void FQTermImageFlow::showStatusMessage(const QString &message) {

    if (statusBar_) {
      statusBar_->showMessage(message, 3000);
    }
  }

  QFileInfoList FQTermImageFlow::sortedList(const QString &path) {

    QDir sortPath(path);

    if (!sortPath.exists()) {
      sortPath.mkdir(path);
    }

    QStringList filters;
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    for (QList<QByteArray>::iterator it = formats.begin();
         it != formats.end();
         ++it) {
      QString filter("*.");
      filter.append(it->data());
      filters << filter;
    }
    sortPath.setNameFilters(filters);

    return (sortPath.entryInfoList(QDir::Files | QDir::Readable | QDir::NoSymLinks, QDir::Time));
  }

  // FQTermImageFlow events.
  void FQTermImageFlow::showEvent(QShowEvent *event) {

    emit isTrashEmpty();

    if (imageFlow_->count() <= 0) {
      loadImages(STAT_UNKNOWN);
      move((desktopSize().width() - width()) / 2.0, (desktopSize().height() - height()) / 2.0);
    } else {
    // TODO: insert new images
	loadImages(STAT_ALL);
	}
  }

  void FQTermImageFlow::closeEvent(QCloseEvent *event) {

	hide();
  }

  class ImageMenu::Private {
  public:
    int emblemStatus;
    bool dustbinStatus;
    bool saveStatus;
    bool clearStatus;

    QString tipMessage;

    QRect rectLike;
    QRect rectTrash;
    QRect rectNew;
    QRect rectRecover;
    QRect rectEmblems;

    QRect rectClear;
    QRect rectSave;
    QRect rectDustbin;

    QPixmap pixmapLike, pixmapLikeGray;
    QPixmap pixmapTrash, pixmapTrashGray;
    QPixmap pixmapNew, pixmapNewGray;
    QPixmap pixmapRecover, pixmapRecoverGray;

    QPixmap pixmapClear, pixmapClearGray;
    QPixmap pixmapSave, pixmapSaveGray;
    QPixmap pixmapDustbin, pixmapDustbinGray;
  };

  //ImageMenu
  ImageMenu::ImageMenu(QWidget *parent)
    : fh(new ImageMenu::Private) {

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Shadow);
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);
    setFixedHeight(60);

    fh->emblemStatus = -2;
    fh->dustbinStatus = false;
    fh->saveStatus = false;
    fh->clearStatus = false;

    fh->tipMessage = "";

    fh->rectLike = QRect(50, 14, 32, 32);
    fh->rectTrash = QRect(100, 14, 32, 32);
    fh->rectNew = QRect(150, 14, 32, 32);
    fh->rectRecover = QRect(200, 14, 32, 32);
    fh->rectEmblems = QRect(50, 14, 232, 32);

    fh->rectClear = QRect(300, 14, 32, 32);
    fh->rectSave = QRect(350, 14, 32, 32);
    fh->rectDustbin = QRect(400, 14, 32, 32);

    fh->pixmapLike = QPixmap(iconPath("emblem-like.png"));
    fh->pixmapLikeGray = QPixmap(iconPath("emblem-like-gray.png"));

    fh->pixmapTrash = QPixmap(iconPath("emblem-trash.png"));
    fh->pixmapTrashGray = QPixmap(iconPath("emblem-trash-gray.png"));

    fh->pixmapNew = QPixmap(iconPath("emblem-new.png"));
    fh->pixmapNewGray = QPixmap(iconPath("emblem-new-gray.png"));

    fh->pixmapRecover = QPixmap(iconPath("emblem-recover.png"));
    fh->pixmapRecoverGray = QPixmap(iconPath("emblem-recover-gray.png"));

    fh->pixmapClear = QPixmap(iconPath("clear-state.png"));
    fh->pixmapClearGray = QPixmap(iconPath("clear-state-gray.png"));

    fh->pixmapSave = QPixmap(iconPath("save-state.png"));
    fh->pixmapSaveGray = QPixmap(iconPath("save-state-gray.png"));

    fh->pixmapDustbin = QPixmap(iconPath("trash-state.png"));
    fh->pixmapDustbinGray = QPixmap(iconPath("trash-state-gray.png"));
  }

  ImageMenu::~ImageMenu() {

    delete fh;
    fh = NULL;
  }

  void ImageMenu::updateEmblems(const int status) {

    if (fh->emblemStatus != status) {
      update(fh->rectEmblems);
    }

    fh->emblemStatus = status;
  }

  void ImageMenu::updateClear(const bool hasOrNot) {

    if (fh->clearStatus != hasOrNot) {
      update(fh->rectClear);
    }

    fh->clearStatus = hasOrNot;
  }

  void ImageMenu::updateSave(const bool hasOrNot) {

    if (fh->saveStatus != hasOrNot) {
      update(fh->rectSave);
    }

    fh->saveStatus = hasOrNot;
  }

  void ImageMenu::updateDustbin(const bool fullOrNot) {

    if (fh->dustbinStatus != fullOrNot) {
      update(fh->rectDustbin);
    }

    fh->dustbinStatus = fullOrNot;
  }

  void ImageMenu::paintEvent(QPaintEvent *event) {

    QPainter p(this);

    p.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);

    if (fh->emblemStatus == STAT_LIKE) {
      p.drawPixmap(fh->rectLike, fh->pixmapLike);
    } else {
      p.drawPixmap(fh->rectLike, fh->pixmapLikeGray);
    }

    if (fh->emblemStatus == STAT_TRASH) {
      p.drawPixmap(fh->rectTrash, fh->pixmapTrash);
    } else {
      p.drawPixmap(fh->rectTrash, fh->pixmapTrashGray);
    }

    if (fh->emblemStatus == STAT_NEW) {
      p.drawPixmap(fh->rectNew, fh->pixmapNew);
    } else {
      p.drawPixmap(fh->rectNew, fh->pixmapNewGray);
    }

    if (fh->emblemStatus == STAT_RECOVER) {
      p.drawPixmap(fh->rectRecover, fh->pixmapRecover);
    } else {
      p.drawPixmap(fh->rectRecover, fh->pixmapRecoverGray);
    }

    if (fh->clearStatus == true) {
      p.drawPixmap(fh->rectClear, fh->pixmapClear);
    } else {
      p.drawPixmap(fh->rectClear, fh->pixmapClearGray);
    }

    if (fh->saveStatus == true) {
      p.drawPixmap(fh->rectSave, fh->pixmapSave);
    } else {
      p.drawPixmap(fh->rectSave, fh->pixmapSaveGray);
    }

    if (fh->dustbinStatus == true) {
      p.drawPixmap(fh->rectDustbin, fh->pixmapDustbin);
    } else {
      p.drawPixmap(fh->rectDustbin, fh->pixmapDustbinGray);
    }

    p.end();
  }

  void ImageMenu::mouseReleaseEvent(QMouseEvent *event) {

    if (event->button() == Qt::LeftButton
      || event->button() == Qt::RightButton) {

      if (fh->rectLike.contains(event->pos())) {
        emit toggleFlowStatus(STAT_LIKE);
      }

      if (fh->rectTrash.contains(event->pos())) {
        emit toggleFlowStatus(STAT_TRASH);
      }

      if (fh->rectClear.contains(event->pos())) {
        emit clearImages();
      }

      if (fh->rectSave.contains(event->pos())) {
        emit saveImages();
      }

      if (fh->rectDustbin.contains(event->pos())) {
        emit recoverImages();
      }
    }
  }

  // ImageFlow: this subclasses PictureFlow
  class ImageFlow::Private {
  public:
    int emblemStatus;
    int likeCount;
    int clearCount;

    QList<QByteArray> itemsTitles;
    QHash<QByteArray, qint64> itemsTitleKeyPairs;
	QList<ImageFlowItem> items;
  };

  ImageFlow::~ImageFlow() {

	delete m;
	m = NULL;
  }

  // construct an emptry pictureflow.
  ImageFlow::ImageFlow(QWidget *parent)
	: PictureFlow(parent), m(new ImageFlow::Private) {

    // initial setup
    setFocusPolicy(Qt::StrongFocus);
    setZoomFactor(120);
	setSlideCount(0);
    m->emblemStatus = -1;
    m->likeCount = 0;
    m->clearCount = 0;
  }

  const ImageFlowItem& ImageFlow::operator[](int index) const {

    return (m->items[index]);
  }

  ImageFlowItem& ImageFlow::operator[](int index) {

    return (m->items[index]);
  }

  const ImageFlowItem& ImageFlow::at(int index) const {

    return (m->items[index]);
  }

  void ImageFlow::add(const ImageFlowItem &item) {

    QByteArray title = item.info().fileName().toLocal8Bit();

    m->items.append(item);
    m->itemsTitles.append(title);
    m->itemsTitleKeyPairs.insert(title, item.key());

    // set the status pixmap
    int index = m->items.indexOf(item);
    m->items[index].setStatus(item.status());

    int count = m->items.count();
    setSlideCount(count);
    setSlide(count - 1, item.image());
  }

  QList<QByteArray>& ImageFlow::itemsTitles() const {

    return (m->itemsTitles);
  }

  void ImageFlow::clear() {

    m->items.clear();
    setSlideCount(0);
    setCurrentSlide(0);
    PictureFlow::clear();
  }

  int ImageFlow::count() const {

    return (m->items.count());
  }

  int ImageFlow::size() const {

    return (m->items.size());
  }

  QFileInfoList& ImageFlow::digest(const int status) {

    int i;
    int count = m->items.size();
    static QFileInfoList result;

    result.clear();

    for (i = 0; i < count; i++) {
      switch (status) {
        case STAT_ALL:
          result.append(m->items[i].info());
          break;
        default:
          if (m->items[i].status() == status) {
            result.append(m->items[i].info());
          }
          break;
      }
    }

    return result;
  }

  void ImageFlow::strip(const int status) {

    int i;
    int count = m->items.count();
    QList<int> dirtyIndexes;

    // We cannot delete items in a loop because
    // items count will change while deleting.
    // A better method is to use another list
    // to record 'dirty' items, and by looping this
    // dirty list, we can correctly delete those
    // dirty items successfully.
    for (i = 0; i < count; i++) {
      if (m->items[i].status() == status) {
        dirtyIndexes.append(i);
      }
    }

    if (!dirtyIndexes.isEmpty()) {

      for (i = 0; i < dirtyIndexes.count(); i++) {
        // Note: dirtyIndexes[i] - i: the actual
        // position of the dirty item.
        m->items.removeAt(dirtyIndexes[i] - i);
      }

      dirtyIndexes.clear();
      reorder(sort(STAT_ALL));

      for (i = 0; i < m->items.count(); i++) {
        setSlide(i, m->items[i].image());
      }

      setSlideCount(i);
      setCurrentSlide(0);

      switch(status) {
        case STAT_LIKE:
          m->likeCount = 0;
          emit saveStatus(false);
          emit emblemStatus(STAT_UNKNOWN);
          break;
        case STAT_TRASH:
          m->clearCount = 0;
          emit clearStatus(false);
          emit emblemStatus(STAT_UNKNOWN);
          break;
        default:
          break;
      }
    }
  }

  QList<qint64> ImageFlow::sort(const int status) {

    int i;
    int count = m->items.count();
    QList<qint64> itemKeys;

    switch (status) {
      case STAT_TITLE:
        qStableSort(m->itemsTitles.begin(), m->itemsTitles.end(), qLess<QByteArray>());
        for (i = 0; i < m->itemsTitles.count(); i++) {
          itemKeys.append(m->itemsTitleKeyPairs.value(m->itemsTitles[i]));
        }
        break;
      default:
        for (i = 0; i < count; i++) {
          if (m->items[i].status() == status) {
            itemKeys.prepend(m->items[i].key());
          } else {
            itemKeys.append(m->items[i].key());
          }
        }
        break;
    }

    // this might be unnecessary here.
    return itemKeys;
  }

  bool ImageFlow::reorder(const QList<qint64>& itemsKey) {

	int items_count = m->items.size();

	if (itemsKey.size() != items_count) {

      return (false);
    }

	// Collect Items Key
	QList<qint64> currentItemsKey;
	for (int i = 0; i < items_count; i++) {

      currentItemsKey.append(m->items.at(i).key());
    }

    if (currentItemsKey == itemsKey) {
      // if identical, we don't sort.
      return (false);
    }

	// Swap Items
	for (int i=0; i < items_count; i++) {

      int index = currentItemsKey.indexOf(itemsKey.at(i));
      if (i == index) {
        continue;
      }

      QImage imgA = m->items[i].image();
      QImage imgB = m->items[index].image();

      setSlide(index, imgA);
      setSlide(i, imgB);

      m->items.swap(i, index);
      currentItemsKey.swap(i, index);	
	}

	update();
    rehash();
	return(true);
  }

  void ImageFlow::rehash(void) {

    int count = m->items.count();

    if (count <= 0) {
      return;
    }

    int i;

    m->itemsTitles.clear();
    m->itemsTitleKeyPairs.clear();

    qint64 k;
    QByteArray b;

    for (i = 0; i < count; i++) {
      k = m->items[i].key();
      b = m->items[i].info().fileName().toLocal8Bit();
      m->itemsTitles.append(b);
      m->itemsTitleKeyPairs.insert(b, k);
    }
  }

  void ImageFlow::toggleStatus(const int status) {

    int index = currentSlide();
    int count = m->items.count();

    if (index >= 0 && index < count) {

      setUpdatesEnabled(false);
      if (m->items[index].status() != status) {

        switch (status) {
          case STAT_LIKE:
            m->likeCount++;
            if (m->items[index].status() == STAT_TRASH) {
              m->clearCount--;
            }
            emit saveStatus(true);
            break;
          case STAT_TRASH:
            m->clearCount++;
            if (m->items[index].status() == STAT_LIKE) {
              m->likeCount--;
            }
            emit clearStatus(true);
            break;
          default:
            break;
        }

        m->items[index].setStatus(status);
        emit statusMessage(tr("Tagged as ") + tr(fqloraTagStatus[status]));

      } else {

        switch (status) {
          case STAT_LIKE:
            m->likeCount--;
            break;
          case STAT_TRASH:
            m->clearCount--;
            break;
          default:
            break;
        }

        m->items[index].setStatus(STAT_UNKNOWN);
        emit statusMessage("Tag cleared");
      }

      if (m->likeCount <= 0) {
        emit saveStatus(false);
      } else {
        emit saveStatus(true);
      }

      if (m->clearCount <= 0) {
        emit clearStatus(false);
      } else {
        emit clearStatus(true);
      }

      setUpdatesEnabled(true);
    }
  }

  void ImageFlow::setCurrentImage(const int index) {

    int count = size();

    if (count > 0 && index > 0 && index < count) {

      setCurrentSlide(index);
    }
  }

// events
  void ImageFlow::paintEvent(QPaintEvent *event) {

	PictureFlow::paintEvent(event);

	if (slideCount() < 1) {
      return;
    }

	QPainter p(this);

	// White Pen for File Info
	p.setPen(Qt::gray);

	int cw = width() / 2;
	int wh = height();

	ImageFlowItem& item = m->items[currentSlide()];

	// Draw File Name if it's not empty
    QString title = item.info().fileName();
    if (!title.isEmpty()) {

      p.setFont(QFont(p.font().family(), p.font().pointSize() + 1, QFont::Bold));
      p.drawText(cw - (QFontMetrics(p.font()).width(title) / 2), wh - 20, title);
    }

    p.end();

    if (m->emblemStatus != item.status()) {
      emit emblemStatus(item.status());
    }

    m->emblemStatus = item.status();
  }

  // ImageFlowItem
  class ImageFlowItem::Private {
  public:
	QString filePath;
    QFileInfo info;
	QString comment;
	QImage image;
    int status;
	qint64 key;

  };

  ImageFlowItem::~ImageFlowItem() {

	delete m;
	m = NULL;
  }

  // constructs an empty item
  ImageFlowItem::ImageFlowItem(QObject *parent)
	: QObject(parent), m(new ImageFlowItem::Private) {

	m->key = qHash(QString::number(m->image.cacheKey()) + m->info.fileName() + m->comment);
  }

  ImageFlowItem::ImageFlowItem(const ImageFlowItem &item)
    : QObject(item.parent()), m(new ImageFlowItem::Private) {

    operator=(item);
  }

  // constructs an item with a given image
  ImageFlowItem::ImageFlowItem(const QImage &image, QObject *parent)
	: QObject(parent), m(new ImageFlowItem::Private) {

	m->image = image;
	m->key = qHash(QString::number(m->image.cacheKey()) + m->info.fileName() + m->comment);
  }

  // constructs an image with given image and title
  ImageFlowItem::ImageFlowItem(const QImage &image, const QFileInfo &info, QObject *parent)
	: QObject(parent), m(new ImageFlowItem::Private) {

	m->image = image;
	m->info = info;
	m->key = qHash(QString::number(m->image.cacheKey()) + m->info.fileName() + m->comment);
  }

  // constructs an image with given image, title and comment
  ImageFlowItem::ImageFlowItem(const QImage &image, const QFileInfo &info, const QString &comment, QObject *parent)
	: QObject(parent), m(new ImageFlowItem::Private) {

	m->image = image;
	m->info = info;
	m->comment = comment;
	m->key = qHash(QString::number(m->image.cacheKey()) + m->info.fileName() + m->comment);
  }

  // constructs an image with given image, title, type, size, time and state
  ImageFlowItem::ImageFlowItem(const QImage &image, const QFileInfo &info, const QString& comment, const int status, QObject *parent)
    : QObject(parent), m(new ImageFlowItem::Private) {

    m->image = image;
    m->info = info;
    m->comment = comment;
    m->status = status;

    m->key = qHash(QString::number(m->image.cacheKey()) + m->info.fileName() + m->comment);
  }

  // funcs and slots
  ImageFlowItem& ImageFlowItem::operator=(const ImageFlowItem &item) {

    m->info = item.m->info;
    m->comment = item.m->comment;
    m->key = item.m->key;

    if (item.m->filePath.isEmpty()) {
      m->image = item.m->image;
    } else {
      setImage(item.m->filePath, m->key);
    }

    return (*this);
  }

  bool ImageFlowItem::operator==(const ImageFlowItem& item) const {

	return (item.m->key == m->key);
  }

  bool ImageFlowItem::operator!=(const ImageFlowItem& item) const {

	return (item.m->key != m->key);
  }

  const QString& ImageFlowItem::comment() const {

	return (m->comment);
  }

  const QImage& ImageFlowItem::image() const {

	return (m->image);
  }

  int ImageFlowItem::status(void) const {

    return (m->status);
  }

  const QFileInfo& ImageFlowItem::info(void) const {

    return (m->info);
  }

  qint64 ImageFlowItem::key() const {

	return (m->key);
  }

  void ImageFlowItem::setInfo(const QFileInfo& info) {

	m->info = info;
	m->key = qHash(QString::number(m->image.cacheKey()) + m->info.fileName() + m->comment);
  }

  void ImageFlowItem::setComment(const QString &comment) {

	m->comment = comment;
	m->key = qHash(QString::number(m->image.cacheKey()) + m->info.fileName() + m->comment);
  }

  void ImageFlowItem::setImage(const QImage &image) {

	m->image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	m->key = qHash(QString::number(m->image.cacheKey()) + m->info.fileName() + m->comment);
  }

  void ImageFlowItem::setImage(const QString &filePath, int size) {

	m->filePath = filePath;
	m->key = size;

	QTimer::singleShot(1000, this, SLOT(loadImage()));
  }

  void ImageFlowItem::setStatus(const int status) {

    m->status = status;
  }

  void ImageFlowItem::loadImage() {

	int imageHeight = m->key;

	if (!m->image.load(m->filePath)) {
	  QFileIconProvider iconProvider;
	  m->image = iconProvider.icon(QFileInfo(m->filePath)).pixmap(imageHeight).toImage();
	} else {
	  m->image = resizeImage(m->image, imageHeight);
	}

	m->filePath.clear();
	m->key = qHash(QString::number(m->image.cacheKey()) + m->info.fileName() + m->comment);
  }

  QImage ImageFlowItem::resizeImage(const QImage &image, int size) {

    if (size == image.width() && size == image.height()) {
      return image;
    }

    double scaleWidth = size / (double)image.width();
    double scaleHeight = size / (double)image.height();
    double smaller = qMin(scaleWidth, scaleHeight);

    int w = (int) qRound(smaller * image.width());
    int h = (int) qRound(smaller * image.height());

    return (image.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
  }

}  // namespace FQTerm

#include "imageviewer.moc"
