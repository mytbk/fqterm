// SPDX-License-Identifier: GPL-2.0-or-later

//WARNING this is not meant for use outside this unit!


#ifndef FQTERM_POPUP_MESSAGE_H
#define FQTERM_POPUP_MESSAGE_H

#include "overlayWidget.h"

// #include <qbitmap.h>
// #include <qlayout.h>
// #include <qpixmap.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QTimerEvent>
#include <QFrame>
#include <QBitmap>
#include <QPixmap>

namespace FQTerm {
/**
 * @class PopupMessage
 * @short Widget that animates itself into a position relative to an anchor widget
 */
class PopupMessage: public OverlayWidget {
  Q_OBJECT;
 public:
  /**
   * @param anchor  : which widget to tie the popup widget to.
   * @param timeout : how long to wait before auto closing. A value of 0 means close
   *                  only on pressing the closeButton or close() is called.
   */
  PopupMessage(QWidget *parent_, QWidget *anchor, int timeout = 5000
               /*milliseconds*/);

  enum MaskEffect {
    Plain, Slide, Dissolve
  };

  void addWidget(QWidget *widget);
  void showCloseButton(const bool show);
  void showCounter(const bool show);
  void setImage(const QString &location_);
  void setMaskEffect(const MaskEffect type) {
    maskEffect_ = type;
  }
  void setText(const QString &text);
  void setTimeout(const int time) {
    timeout_ = time;
  }

 public slots:
  void close();
  void display();

 protected:
  void timerEvent(QTimerEvent*);
  void countDown();

  /**
   * @short Gradually show widget by dissolving from background
   */
  void dissolveMask();

  /**
   * @short instantly display widget
   */
  void plainMask();

  /**
   * @short animation to slide the widget into view
   */
  void slideMask();

 private:
  QVBoxLayout *layout_;
  QFrame *countDownFrame_;
  QWidget *anchor_;
  QWidget *parent_;
  QBitmap maskBitmap_;
  MaskEffect maskEffect_;

  int dissolveSize_;
  int dissolveDelta_;

  int offset_;
  int counter_;
  int stage_;
  int timeout_;
  int timerId_;

  bool isCounterShown_;
};

}  // namespace FQTerm

#endif  // FQTERM_POPUP_MESSAGE_H
