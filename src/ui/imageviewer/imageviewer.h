
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

#ifndef IMAGE_VIERWER_H
#define IMAGE_VIERWER_H

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

class QString;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QMenuBar;
class QComboBox;

#include "pictureflow.h"
#include "fqtermimage.h"

namespace FQTerm {

#define DISP_MARGIN				(10.0) // show a margin
#define PIXMAP_CACHESIZE		(5120) // in KBytes
#define ZOOMIN_FACTOR			(1.15)
#define ZOOMOUT_FACTOR			(0.85)
#define ROTLEFT_DEG				(-90.0)
#define ROTRIGHT_DEG			(90.0)
#define IMG_CHUNK				(16384) // in bytes

#define IMAGE_BROWSER_NAME      "FQLora [Image Companion for FQTerm]"
#define POOL_SOURCE				"pool/"
#define SHADOW_SOURCE			"shadow-cache/"
#define TRASH_SOURCE            ".Trash/"

#define EMBLEM_LIKE             "emblem-like.png"
#define EMBLEM_NEW              "emblem-new.png"
#define EMBLEM_RECOVER          "emblem-recover.png"
#define EMBLEM_TRASH            "emblem-trash.png"
#define EMBLEM_TITLE            "emblem-title.png"
 
  class FQTermConfig;
  class FQTermFileDialog;


  class ImageFlow;
  class ImageMenu;
  class ImageFlowItem;

  class FQTermImageFlow: public FQTermImage {
	Q_OBJECT;

    public:
	FQTermImageFlow(FQTermConfig *, QWidget *, Qt::WindowFlags);
	~FQTermImageFlow();

	/* don't modify the funcs below */
	/* they are for API compatibiliy */
	void adjustItemSize();
	void scrollTo(const QString &);
	void updateImage(const QString &);
	/*********************************/

        public slots:
            void saveImages(void);
            void clearImages(void);
            void trashAllImages(void);
            void recoverImages(void);
            void showStatusMessage(const QString &message);

    private:
            ImageFlow *imageFlow_;
            ImageMenu *imageMenu_;
            QStatusBar *statusBar_;
            FQTermConfig *config_;

            QFileInfoList sortedList(const QString &path);
            const QString& poolSource(void) const;
            const QString& trashSource(void) const;

            private slots:
                void loadImages(const int status);
                void reshuffleImages(const int status);
                void checkTrashState(void);

    signals:
                void statusMessage(const QString &message);
                void isTrashEmpty(void);
                void trashStatus(const bool fullOrNot);
                void saveStatus(const bool hasOrNot);
                void clearStatus(const bool hasOrNot);

    protected:
                void showEvent(QShowEvent *event);
                void closeEvent(QCloseEvent *event);
    };


  class ImageMenu: public QLabel {
    Q_OBJECT;

  public:
    ImageMenu(QWidget *parent = 0);
    ~ImageMenu();

  public slots:
    void updateEmblems(const int status);
    void updateDustbin(const bool fullOrNot);
    void updateSave(const bool hasOrNot);
    void updateClear(const bool hasOrNot);

  signals:
    void toggleFlowStatus(const int status);
    void recoverImages(void);
    void saveImages(void);
    void clearImages(void);

  private:
    class Private;
    Private *fh;

  protected:
    void paintEvent(QPaintEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
  };

  class ImageFlow: public PictureFlow {
	Q_OBJECT;
    Q_PROPERTY(QList<QByteArray> itemsTitles READ itemsTitles);

  public:
	ImageFlow(QWidget *parent);
	~ImageFlow();

    const ImageFlowItem& operator[](int index) const;
    ImageFlowItem& operator[](int index);
    const ImageFlowItem& at(int index) const;

    QList<QByteArray>& itemsTitles(void) const;

    QFileInfoList& digest(const int status);
    void strip(const int status);
    QList<qint64> sort(const int status);
    bool reorder(const QList<qint64>& itemsKey);

  public slots:
    void add(const ImageFlowItem &item);
    void clear(void);
    int count(void) const;
    int size(void) const;
    void rehash(void);
    void toggleStatus(const int status);
    void setCurrentImage(const int index);

  private:
	class Private;
	Private *m;

  signals:
    void statusMessage(const QString &message);
    void emblemStatus(const int status);
    void saveStatus(const bool hasOrNot);
    void clearStatus(const bool hasOrNot);
    void titlesChanged(QList<QByteArray> &titles);

  protected:
    void paintEvent(QPaintEvent *event);
  };

  class ImageFlowItem: public QObject {
	Q_OBJECT;

	Q_PROPERTY(qint64 key READ key);
    Q_PROPERTY(int status READ status WRITE setStatus);
	Q_PROPERTY(QImage image READ image WRITE setImage);
	Q_PROPERTY(QFileInfo info READ info WRITE setInfo);
	Q_PROPERTY(QString comment READ comment WRITE setComment);

  public:
	ImageFlowItem(QObject *parent = 0);
    ImageFlowItem(const ImageFlowItem &item);
	ImageFlowItem(const QImage &image, QObject *parent = 0);
	ImageFlowItem(const QImage &image, const QFileInfo &info, QObject *parent = 0);
    ImageFlowItem(const QImage &image, const QFileInfo &info, const int state, QObject *parent = 0);
	ImageFlowItem(const QImage &image, const QFileInfo &info, const QString &comment, QObject *parent = 0);
    ImageFlowItem(const QImage &image, const QFileInfo &info, const QString& comment, const int status, QObject *parent = 0);
	~ImageFlowItem();

	const QFileInfo& info(void) const;
	const QString& comment(void) const;
	const QImage& image(void) const;
    int status(void) const;
	qint64 key(void) const;

    ImageFlowItem& operator=(const ImageFlowItem &item);
    bool operator==(const ImageFlowItem &item) const;
    bool operator!=(const ImageFlowItem &item) const;

  public slots:
    void setStatus(const int status);
	void setInfo(const QFileInfo &info);
	void setComment(const QString &comment);
	void setImage(const QImage &image);
	void setImage(const QString &filePath, int size);
	
  private slots:
	void loadImage(void);
	
  private:
	QImage resizeImage(const QImage &image, int size);
	class Private;
	Private *m;

  };


}  // namespace FQTerm

#endif  // IMAGE_VIERWER_H
