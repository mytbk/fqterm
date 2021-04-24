// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_OSD_MESSAGE_H
#define FQTERM_OSD_MESSAGE_H

#include <QPixmap>
#include <QWidget>
#include <QHideEvent>

namespace FQTerm {

/**
 * A widget that displays messages in the top-left corner.
 */
class PageViewMessage: public QWidget {
  Q_OBJECT;
 public:
  PageViewMessage(QWidget *parent_);

  enum Alignment {
    TopLeft, TopRight, BottomLeft, BottomRight
  };
  enum Icon {
    None, Info, Warning, Error, Find
  };
  void display(const QString &message, Icon icon = Info, int durationMs = 4000,
               QPoint pos = QPoint(10, 10), Alignment ali = TopLeft);
  void display(const QString &message, QPoint pos, Icon icon = Info);
  QRect displayCheck(const QString &message, Icon icon = Info);
  public slots:
    void showText(const QString &message){
      display(message);
    }
 protected:
  void paintEvent(QPaintEvent *e);
  void mousePressEvent(QMouseEvent *e);
  QRect displayImpl(const QString &message, Icon icon = Info,
                    bool check = false, int durationMs = 4000,
                    QPoint pos = QPoint(10, 10), Alignment ali = TopLeft);
  void hideEvent(QHideEvent * event);
 private:
  QPixmap pixmap_;
  QTimer *timer_;
  QString message_;
signals:
  void hideAt(const QRect& rect);
};

}  // namespace FQTerm

#endif  // FQTERM_OSD_MESSAGE_H
