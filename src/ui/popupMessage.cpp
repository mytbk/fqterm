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

#include <QVBoxLayout>
#include <QPixmap>
#include <QHBoxLayout>
#include <QTimerEvent>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QFont>
#include <QPushButton>
#include <QPainter>

#include "popupMessage.h"
#include "fqterm_trace.h"

namespace FQTerm {

PopupMessage::PopupMessage(QWidget *parent_, QWidget *anchor, int timeout)
    : OverlayWidget(parent_, anchor),
      anchor_(anchor),
      parent_(parent_),
      maskEffect_(Slide),
      dissolveSize_(0),
      dissolveDelta_(-1),
      offset_(0),
      counter_(0),
      stage_(1),
      timeout_(timeout),
      isCounterShown_(true) {
  setFrameStyle(QFrame::Panel | QFrame::Raised);
  setFrameShape(QFrame::StyledPanel);
  //     overrideWindowFlags( Qt::WX11BypassWM );

  QHBoxLayout *hbox;
  QLabel *label;
  QLabel *alabel;
  //KActiveLabel *alabel;

  layout_ = new QVBoxLayout(this); //, 9 /*margin*/, 6 /*spacing*/ );
  layout_->setMargin(9);
  layout_->setSpacing(6);

  hbox = new QHBoxLayout; //( layout_, 12 );
  hbox->setMargin(12);

  layout_->addLayout(hbox);

  hbox->addWidget(countDownFrame_ = new QFrame(this)); //, "counterVisual" ) );
  countDownFrame_->setObjectName("counterVisual");
  countDownFrame_->setFixedWidth(fontMetrics().width("X"));
  countDownFrame_->setFrameStyle(QFrame::Plain | QFrame::Box);
  QPalette tmp_palette;
  tmp_palette.setColor(countDownFrame_->foregroundRole(), palette().color
                       (QPalette::Window).dark());
  countDownFrame_->setPalette(tmp_palette);
  //     m_countdownFrame->setPaletteForegroundColor( paletteBackgroundColor().dark() );

  label = new QLabel(this); //, "image" );
  label->setObjectName("image");
  hbox->addWidget(label);
  label->hide();

  alabel = new QLabel("Details of the tasks: ", this); //, "label");
  alabel->setObjectName("label");
  //alabel = new KActiveLabel( this, "label" );
  //alabel->setTextFormat( Qt::RichText );
  alabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

  hbox->addWidget(alabel);

  hbox = new QHBoxLayout;

  layout_->addLayout(hbox);

  hbox->addItem(new QSpacerItem(4, 4, QSizePolicy::Expanding,
                                QSizePolicy::Preferred));
  //FIXME: add icon
  QPushButton *tmp_button = new QPushButton("Close", this);
  tmp_button->setObjectName("closeButton");
  hbox->addWidget(tmp_button); //, "closeButton" ) );

  FQ_VERIFY(connect(findChild < QPushButton * > ("closeButton"), SIGNAL(clicked()),
                  SLOT(close())));
}

void PopupMessage::addWidget(QWidget *widget) {
  layout_->addWidget(widget);
  adjustSize();
}

void PopupMessage::showCloseButton(const bool show) {
  findChild < QPushButton * > ("closeButton")->setVisible(show);
  adjustSize();
}

void PopupMessage::showCounter(const bool show) {
  isCounterShown_ = show;
  findChild < QFrame * > ("counterVisual")->setVisible(show);
  adjustSize();
}

void PopupMessage::setText(const QString &text) {
  findChild < QLabel * > ("label")->setText(text);
  adjustSize();
}

void PopupMessage::setImage(const QString &location_) {
  findChild < QLabel * > ("image")->setPixmap(QPixmap(location_));
  adjustSize();
}


////////////////////////////////////////////////////////////////////////
//     Public Slots
////////////////////////////////////////////////////////////////////////

void PopupMessage::close() { //SLOT
  stage_ = 3;
  killTimer(timerId_);
  timerId_ = startTimer(6);
}

void PopupMessage::display() { //SLOT
  dissolveSize_ = 24;
  dissolveDelta_ = -1;

  if (maskEffect_ == Dissolve) {
    // necessary to create the mask
    maskBitmap_ = QBitmap(width(), height());
    // make the mask empty and hence will not show widget with show() called below
    dissolveMask();
    timerId_ = startTimer(1000 / 30);
  } else {
    timerId_ = startTimer(6);
  }
  show();
}

////////////////////////////////////////////////////////////////////////
//     Protected
////////////////////////////////////////////////////////////////////////

void PopupMessage::timerEvent(QTimerEvent*) {
  switch (maskEffect_) {
    case Plain:
      plainMask();
      break;

    case Slide:
      slideMask();
      break;

    case Dissolve:
      dissolveMask();
      break;
  }
}

void PopupMessage::countDown() {
  if (!timeout_) {
    killTimer(timerId_);
    return ;
  }

  QFrame * &h = countDownFrame_;

  if (counter_ < h->height() - 3) {
    QPainter(h).fillRect(2, 2, h->width() - 4, counter_, palette().color
                         (QPalette::Active, QPalette::Highlight));
  }

  if (!testAttribute(Qt::WA_UnderMouse)) {
    counter_++;
  }

  if (counter_ > h->height()) {
    stage_ = 3;
    killTimer(timerId_);
    timerId_ = startTimer(6);
  } else {
    killTimer(timerId_);
    timerId_ = startTimer(timeout_ / h->height());
  }
}

void PopupMessage::dissolveMask() {
  if (stage_ == 1) {
    //repaint( false );
    QPainter maskPainter(&maskBitmap_);

    maskBitmap_.fill(Qt::black);

    maskPainter.setBrush(Qt::white);
    maskPainter.setPen(Qt::white);
    maskPainter.drawRect(maskBitmap_.rect());

    dissolveSize_ += dissolveDelta_;

    if (dissolveSize_ > 0) {
      //maskPainter.setCompositionMode( Qt::EraseROP );
      //FIXME: QRubberBand

      int x, y, s;
      const int size = 16;

      for (y = 0; y < height() + size; y += size) {
        x = width();
        s = dissolveSize_ * x / 128;

        for (; x > size; x -= size, s -= 2) {
          if (s < 0) {
            break;
          }

          maskPainter.drawEllipse(x - s / 2, y - s / 2, s, s);
        }
      }
    } else if (dissolveSize_ < 0) {
      dissolveDelta_ = 1;
      killTimer(timerId_);

      if (timeout_) {
        timerId_ = startTimer(40);
        stage_ = 2;
      }
    }

    setMask(maskBitmap_);
  } else if (stage_ == 2) {
    countDown();
  } else {
    deleteLater();
  }
}


void PopupMessage::plainMask() {
  switch (stage_) {
    case 1:
      // Raise
      killTimer(timerId_);
      if (timeout_) {
        timerId_ = startTimer(40);
        stage_ = 2;
      }

      break;

    case 2:
      // Counter
      countDown();
      break;

    case 3:
      // Lower/Remove
      deleteLater();
  }
}


void PopupMessage::slideMask() {
  switch (stage_) {
    case 1:
      //raise
      move(0, parent_->y() - offset_);

      offset_++;
      if (offset_ > height()) {
        killTimer(timerId_);

        if (timeout_) {
          timerId_ = startTimer(40);
          stage_ = 2;
        }
      }

      break;

    case 2:
      //fill in pause timer bar
      countDown();
      break;

    case 3:
      //lower
      offset_--;
      move(0, parent_->y() - offset_);

      if (offset_ < 0) {
        deleteLater();
      }
  }
}

}  // namespace FQTerm

#include "popupMessage.moc"
