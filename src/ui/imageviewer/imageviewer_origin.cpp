/* FQTerm image viewer origin.
 */

#include <QAction>
#include <QBuffer>
#include <QCursor>
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
#include "imageviewer_origin.h"

namespace FQTerm {

  void FQTermImageOrigin::onChange(const QModelIndex & index) {

    if (!model_->isDir(index)) {

      if (!isHidden())
        canvas_->hide();

      QString exifInfo = QString::fromStdString(exifExtractor_->extractExifInfo(model_->filePath(tree_->currentIndex()).toLocal8Bit().data()));
      bool resized = false;

      if (exifInfo != "") {

        if (!isExifTableShown_) {
          adjustLayout(true);
          isExifTableShown_ = true;
          resized = true;
        }

        updateExifInfo();
      } else {

        if (isExifTableShown_) {
          adjustLayout(false);
          isExifTableShown_ = false;
          resized = true;
        }
      }

      QString path  = QDir::toNativeSeparators(model_->filePath(index));

      if (path.endsWith(QDir::separator()))
        path.chop(1);

      canvas_->loadImage(path, !resized);

      //    canvas_->autoAdjust();
      if (!isHidden())
        canvas_->show();
    }
  }

  FQTermImageOrigin::~FQTermImageOrigin() {
    delete menuBar_;
    delete canvas_;
    delete tree_;
    delete model_;
  }

  FQTermImageOrigin::FQTermImageOrigin(FQTermConfig * config, QWidget *parent,
    Qt::WindowFlags wflag) :
    FQTermImage(parent, wflag),
    config_(config),
    isExifTableShown_(false) {

    setWindowTitle(tr("FQTerm Image Viewer"));
    ItemDelegate* itemDelegate = new ItemDelegate;
    exifExtractor_ = new ExifExtractor;
    exifTable_ = new ExifTable(this);
    exifTable_->setTextFormat(Qt::RichText);
    exifTable_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    canvas_ = new FQTermCanvas(config, this, 0);
    canvas_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    model_ = new ImageViewerDirModel;
    tree_ = new QTreeView;

    
    tree_->setModel(model_);
    tree_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    adjustItemSize();
    tree_->setItemDelegate(itemDelegate);
    tree_->setColumnWidth(0, 150);
    tree_->setColumnWidth(1, 0);
   
    tree_->hideColumn(0);

    tree_->setUniformRowHeights(true);
    tree_->setWordWrap(true);
    //tree_->header()->setResizeMode(0, QHeaderView::Fixed);
    //tree_->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    //tree_->header()->resizeSection(0, 150);
    //tree_->header()->resizeSection(1, 1);
    comboBox_ = new QComboBox(this);
    comboBox_->addItem(tr("Sort by name"), QDir::Name);
    comboBox_->addItem(tr("Sort by time"), QDir::Time);
    comboBox_->addItem(tr("Sort by size"), QDir::Size);
    comboBox_->addItem(tr("Sort by type"), QDir::Type);

    FQ_VERIFY(connect(comboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(sortFileList(int))));

    comboBox_->setCurrentIndex(1);

    layout_ = new QGridLayout;
    menuBar_ = new QMenuBar(this);
    menuBar_->addMenu(canvas_->menu());
    menuBar_->resize(1,1);

    canvas_->ToolBar()->addAction(
      QIcon(getPath(RESOURCE) + ICON_SOURCE + "prev.png"), tr("Previous"),
      this, SLOT(previous()));
    canvas_->ToolBar()->addAction(
      QIcon(getPath(RESOURCE) + ICON_SOURCE + "next.png"), tr("Next"),
      this, SLOT(next()));

    layout_->addWidget(tree_, 0, 0, 12, 1);
    layout_->addWidget(comboBox_, 12, 0, 1, 1);
    layout_->addWidget(canvas_, 0, 1, 12, 10);
    //  layout_->addWidget(exifTable_, 10, 1, 2, 10);
    layout_->addWidget(canvas_->ToolBar(), 12, 1, 1, 10, Qt::AlignHCenter);
    layout_->setColumnMinimumWidth(0, tree_->columnWidth(0) + 150);
    setLayout(layout_);

    /*
      FQ_VERIFY(connect(tree_, SIGNAL(clicked(const QModelIndex &)),
      this, SLOT(onChange(const QModelIndex &))));
    */
    FQ_VERIFY(connect(tree_, SIGNAL(activated(const QModelIndex &)),
        this, SLOT(onChange(const QModelIndex &))));
    FQ_VERIFY(connect(tree_->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection&,
            const QItemSelection&)),
        this, SLOT(selectionChanged(const QItemSelection&,
            const QItemSelection&))));
    FQ_VERIFY(connect(exifTable_, SIGNAL(showExifDetails()),
        this, SLOT(showFullExifInfo())));
  }

  void FQTermImageOrigin::scrollTo(const QString& filename) {

    QString path = QFileInfo(filename).absolutePath();
    model_->refresh();
    tree_->setRootIndex(model_->index(path));
    canvas_->loadImage(filename);

    if (canvas_->isHidden() && !isHidden()) {
      canvas_->show();
    }

    const QModelIndex& index = model_->index(filename);
    tree_->scrollTo(index);
    tree_->setCurrentIndex(index);
  }

  void FQTermImageOrigin::updateImage(const QString& filename) {

    static int i = 0;
    if (++i == 10) {
      model_->refresh(model_->index(filename));
      i = 0;
    }
    canvas_->updateImage(filename);
  }

  void FQTermImageOrigin::previous() {

    const QModelIndex& index = tree_->indexAbove(tree_->currentIndex());
    if (index.isValid()) {
      tree_->setCurrentIndex(index);
      canvas_->loadImage(QDir::toNativeSeparators(model_->filePath(index)));
    }
  }

  void FQTermImageOrigin::next() {

    const QModelIndex& index = tree_->indexBelow(tree_->currentIndex());
    if (index.isValid()) {
      tree_->setCurrentIndex(index);
      canvas_->loadImage(QDir::toNativeSeparators(model_->filePath(index)));
    }
  }

  void FQTermImageOrigin::adjustItemSize() {

    QFontMetrics fm(font());
    ItemDelegate::size_.setWidth(qMax(128, fm.width("WWWWWWWW.WWW")));
    ItemDelegate::size_.setHeight(fm.height() + 150);
  }

  void FQTermImageOrigin::selectionChanged(const QItemSelection & selected,
    const QItemSelection & deselected) {

    onChange(tree_->selectionModel()->currentIndex());
  }

  void FQTermImageOrigin::sortFileList(int index) {

    model_->setSorting(QDir::SortFlag(comboBox_->itemData(index).toInt()));
    QString poolPath = config_->getItemValue("preference", "pool");

    if (poolPath.isEmpty()) {
      poolPath = getPath(USER_CONFIG) + "pool/";
    }

    tree_->setRootIndex(model_->index(poolPath));
  }

  void FQTermImageOrigin::showFullExifInfo() {

    QString exifInfo = QString::fromStdString(exifExtractor_->extractExifInfo(model_->filePath(tree_->currentIndex()).toLocal8Bit().data()));
    QString comment;

    if ((*exifExtractor_)["UserComment"].length() > 8) {

      QString commentEncoding = QString::fromStdString((*exifExtractor_)["UserComment"].substr(0, 8));

      if (commentEncoding.startsWith("UNICODE")) {
        //UTF-16
        QTextCodec* c = QTextCodec::codecForName("UTF-16");
        comment = c->toUnicode((*exifExtractor_)["UserComment"].substr(8).c_str());
      } else if (commentEncoding.startsWith("JIS")) {
        //JIS X 0208
        QTextCodec* c = QTextCodec::codecForName("JIS X 0208");
        comment = c->toUnicode((*exifExtractor_)["UserComment"].substr(8).c_str());
      } else {
        comment = QString::fromStdString((*exifExtractor_)["UserComment"].substr(8));
      }
    }


    QTextEdit* info = new QTextEdit;
    info->setText(exifInfo + tr("Comment : ") + comment + "\n");
    info->setWindowFlags(Qt::Dialog);
    info->setAttribute(Qt::WA_DeleteOnClose);
    info->setAttribute(Qt::WA_ShowModal);
    //  info->setLineWrapMode(QTextEdit::NoWrap);
    info->setReadOnly(true);
    QFontMetrics fm(font());
    info->resize(fm.width("Orientation : 1st row - 1st col : top - left side    "), fm.height() * 20);
    info->show();
  }

  void FQTermImageOrigin::adjustLayout(bool withExifTable) {

    if (withExifTable) {

      layout_->addWidget(canvas_, 0, 1, 11, 10);
      layout_->addWidget(exifTable_, 11, 1, 1, 10, Qt::AlignHCenter);
      if (!isHidden() && exifTable_->isHidden()) {
        exifTable_->show();
      }

      layout_->addWidget(canvas_->ToolBar(), 12, 1, 1, 10, Qt::AlignHCenter);
    } else {
      layout_->addWidget(canvas_, 0, 1, 12, 10);
      layout_->removeWidget(exifTable_);
      exifTable_->hide();
      layout_->addWidget(canvas_->ToolBar(), 12, 1, 1, 10, Qt::AlignHCenter);
    }
  }

  void FQTermImageOrigin::updateExifInfo() {

    exifTable_->clear();

    QString exifInfoToShow = "<table border=\"1\"><tr><td>"
      + tr("Model") + " : " + QString::fromStdString((*exifExtractor_)["Model"]) + "</td><td>"
      + QString::fromStdString((*exifExtractor_)["DateTime"]) + "</td><td>"
      + QString::fromStdString((*exifExtractor_)["Flash"]) + "</td>"
      + "</tr><tr><td>"
      + tr("ExposureTime") + " : " + QString::fromStdString((*exifExtractor_)["ExposureTime"]) + "</td><td>"
      + tr("FNumber") + " : " + QString::fromStdString((*exifExtractor_)["FNumber"]) + "</td><td>"
      + tr("ISO") + " : " + QString::fromStdString((*exifExtractor_)["ISOSpeedRatings"]) + "</td>"
      + "</tr><tr><td>"
      + tr("FocalLength") + " : " + QString::fromStdString((*exifExtractor_)["FocalLength"]) + "</td><td>"
      + tr("MeteringMode") + " : " + QString::fromStdString((*exifExtractor_)["MeteringMode"]) + "</td><td>" 
      + tr("ExposureBias") + " : " + QString::fromStdString((*exifExtractor_)["ExposureBiasValue"]) + "</td></tr></tabel>";

    exifTable_->setText(exifInfoToShow);
    if (!isHidden() && exifTable_->isHidden()) {
      exifTable_->show();
    }
  }

  void FQTermImageOrigin::closeEvent( QCloseEvent *clse ) {

    hide();
    clse->ignore();
    return ;
  }

  ImageViewerDirModel::ImageViewerDirModel(QObject *parent /*= 0*/)
  : QDirModel(parent) {

	  //insertColumn(1);
    QStringList nameFilterList;
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    for (QList<QByteArray>::iterator it = formats.begin();
         it != formats.end();
         ++it) {
      QString filter("*.");
      filter.append(it->data());
      nameFilterList << filter;
    }
    setNameFilters(nameFilterList);
    setFilter(QDir::Files);
  }

  int ImageViewerDirModel::columnCount(const QModelIndex &/*parent*/) const {

    return 2;
  }

  QVariant ImageViewerDirModel::headerData(
    int section, Qt::Orientation orientation, int role) const {
	  //if (section == 0) return QVariant();
    if (role == Qt::DisplayRole) {
	    //      if (section == 1) {
        return QString(tr("Image Preview"));
	//}
    }
    return QDirModel::headerData(section, orientation, role);
  }

  QVariant ImageViewerDirModel::data(const QModelIndex &index, int role) const {
	  //if (index.column() == 0) return QVariant();
    if (role == Qt::DecorationRole) {
      if (isDir(index)) {
        return QVariant();
      }

      QString path  = QDir::toNativeSeparators(filePath(index));
      if (path.endsWith(QDir::separator()))
        path.chop(1);

      QPixmap pixmap;
      bool res = pixmap.load(path);
      if (!res) {
        QList<QByteArray> formats = QImageReader::supportedImageFormats();
        for (QList<QByteArray>::iterator it = formats.begin();
             !res && it != formats.end();
             ++it) {
          res = pixmap.load(path, it->data());
        }
      }

      if (pixmap.height() > 128 || pixmap.width() > 128) {
        return pixmap.scaled(128, 128,
          Qt::KeepAspectRatio, Qt::SmoothTransformation);
      }

      return pixmap;
    } else if (role == Qt::DisplayRole) {
      return fileName(index);
    }/*
       else if (role == Qt::TextAlignmentRole) {
       return Qt::Qt::AlignBottom;
       }*/
    return QVariant();
  }

  QSize ItemDelegate::size_;

  void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option,
    const QModelIndex & index ) const {
	  //if (index.column() == 0) return;
    QStyleOptionViewItemV3 opt = setOptions(index, option);
 
    // prepare
    painter->save();

    // get the data and the rectangles
    const QPixmap& pixmap
      = qvariant_cast<QPixmap>(index.data(Qt::DecorationRole));
    QRect decorationRect = QRect(opt.rect.topLeft(), pixmap.size());
    decorationRect.moveTo(decorationRect.left(), decorationRect.top() + 10);
    const QString& text = index.data(Qt::DisplayRole).toString();
    QFontMetrics fm(painter->font());
    QRect displayRect = QRect(decorationRect.bottomLeft(),
      QSize(fm.width(text),fm.height()));

    QRect checkRect;
    Qt::CheckState checkState = Qt::Unchecked;
    QVariant value = index.data(Qt::CheckStateRole);

    if (value.isValid()) {
      checkState = static_cast<Qt::CheckState>(value.toInt());
#if QT_VERSION >= 0x050000
      checkRect = doCheck(opt, opt.rect, value);
#else
      checkRect = check(opt, opt.rect, value);
#endif
    }

    // do the layout

    //  doLayout(opt, &checkRect, &decorationRect, &displayRect, false);

    // draw the item

    drawBackground(painter, opt, index);
    painter->drawPixmap(decorationRect, pixmap);
    painter->drawText(displayRect, text);

    drawFocus(painter, opt, displayRect);

    // done
    painter->restore();
  }

  void ExifTable::mouseReleaseEvent(QMouseEvent *pEvent) {

    if (pEvent->button() == Qt::LeftButton) {
      emit(showExifDetails());
    }
  }

  ExifTable::ExifTable(QWidget *parent) : QLabel(parent) {

  }

}

#include "imageviewer_origin.moc"
