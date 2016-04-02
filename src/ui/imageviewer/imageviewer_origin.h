///////////////////////////////////////////////////////
//////// the origin image viewer //////////////////////
///////////////////////////////////////////////////////

#ifndef FQTERM_IMAGEVIEWER_ORIGIN_H
#define FQTERM_IMAGEVIEWER_ORIGIN_H

#include <QDesktopWidget>
#include <QDirModel>
#include <QLabel>
#include <QPainter>
#include <QPixmapCache>
#include <QScrollArea>
#include <QStatusBar>
#include <QToolButton>
#include <QToolBar>
#include <QLayout>
#include <QItemDelegate>
#include <QItemSelection>
#include <QTreeView>
#include <QMenuBar>
#include <QComboBox>

#include "pictureflow.h"
#include "fqtermimage.h"

namespace FQTerm
{

    class FQTermCanvas;
    class ExifExtractor;

    class ItemDelegate : public QItemDelegate {
    public:
        static QSize size_;

        ItemDelegate() {
            size_ = QSize(250,200);
        }

        QSize sizeHint (const QStyleOptionViewItem & option, const QModelIndex & index) const {
            return size_;
        }

        void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    };

    class ExifTable : public QLabel {
        Q_OBJECT;

    public:
        ExifTable(QWidget *parent);

    signals:
        void showExifDetails();

    protected:
        void mouseReleaseEvent(QMouseEvent *pEvent);
    };

    class ImageViewerDirModel : public QDirModel {
    public:
        ImageViewerDirModel(QObject *parent = 0);

        int columnCount(const QModelIndex & = QModelIndex()) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role) const;
        QVariant data(const QModelIndex &index, int role) const;
    };

    class FQTermImageOrigin: public FQTermImage {
        Q_OBJECT;

    public:
        FQTermImageOrigin(FQTermConfig * config, QWidget *parent, Qt::WindowFlags wflag);
        ~FQTermImageOrigin();
        void scrollTo(const QString& filename);
        void updateImage(const QString& filename);

        public slots:
            void onChange(const QModelIndex & index);
            void next();
            void previous();
            void adjustItemSize();
            void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
            void sortFileList(int index);
            void showFullExifInfo();
            void adjustLayout(bool withExifTable);
            void updateExifInfo();

    protected:
            void closeEvent(QCloseEvent *clse);

    private:
            FQTermCanvas* canvas_;
            QTreeView* tree_;
            ImageViewerDirModel* model_;
            QMenuBar* menuBar_;
            QComboBox* comboBox_;
            FQTermConfig* config_;
            ExifExtractor* exifExtractor_;
            ExifTable* exifTable_;
            QGridLayout* layout_;
            bool isExifTableShown_;
    };

}

#endif
