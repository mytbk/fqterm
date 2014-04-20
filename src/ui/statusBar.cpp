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

#include <QPaintEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QEvent>
#include <QProgressBar>
#include <QToolButton>
#include <QPushButton>
#include <QPainter>
#include <QPixmap>
#include <QMessageBox>
#include <QApplication>
#include <QToolTip>

#include "fqterm_trace.h"
#include "fqterm_path.h"

#include "popupMessage.h"
#include "progressBar.h"
#include "overlayWidget.h"
#include "statusBar.h"

namespace FQTerm {

//TODO allow for uncertain progress periods

StatusBar *StatusBar::singletonStatusBar_ = 0;

StatusBar::StatusBar(QWidget *parent, const char *name)
    : QWidget(parent) {
  setObjectName(name);
  singletonStatusBar_ = this;
  QBoxLayout *mainlayout = new QHBoxLayout(this); //, 2, /*spacing*/5 );
  mainlayout->setMargin(0);
  mainlayout->setSpacing(5);

  //we need extra spacing due to the way we paint the surrounding boxes
  QBoxLayout *layout = new QHBoxLayout;
  layout->setSpacing(5);
  mainlayout->addLayout(layout);

  m_mainTextLabel = new QLabel(this);
  m_mainTextLabel->setObjectName("mainTextLabel");
  m_mainTextLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
  
  QWidget *mainProgressBarBox = new QWidget(this);
  mainProgressBarBox->setObjectName("progressBox");
  
  QHBoxLayout *mainProgressBarLayout = new QHBoxLayout(mainProgressBarBox);
  mainProgressBarLayout->setContentsMargins(0, 0, 0, 0);
  
  QToolButton *b1 = new QToolButton(mainProgressBarBox);
  b1->setObjectName("cancelButton");
  b1->setContentsMargins(0, 0, 0, 0);
  b1->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
  mainProgressBarLayout->addWidget(b1);
  
  mainProgressBar_ = new QProgressBar(mainProgressBarBox);
  mainProgressBar_->setObjectName("mainProgressBar");
  mainProgressBarLayout->addWidget(mainProgressBar_);
  
  QToolButton *b2 = new QToolButton(mainProgressBarBox);
  b2->setObjectName("showAllProgressDetails");
  b2->setContentsMargins(0,0,0,0);
  b2->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);  
  mainProgressBarLayout->addWidget(b2);
  
  mainProgressBarLayout->setSpacing(2);
  mainProgressBarBox->hide();

  layout->addWidget(mainProgressBarBox);
  layout->addWidget(m_mainTextLabel);
  
  layout->setStretchFactor(m_mainTextLabel, 2);
  layout->setStretchFactor(mainProgressBarBox, 1);

  otherWidgetLayout_ = new QHBoxLayout;
  otherWidgetLayout_->setSpacing(0);
  mainlayout->addLayout(otherWidgetLayout_);

  mainlayout->setStretchFactor(layout, 6);
  mainlayout->setStretchFactor(otherWidgetLayout_, 4);

  b1->setIcon(QPixmap(getPath(RESOURCE) + "pic/messagebox_critical.png"));
  b2->setIcon(QPixmap(getPath(RESOURCE) + "pic/messagebox_info.png"));
  b2->setCheckable(true);
  b1->setToolTip(tr("Abort all background-operations"));
  b2->setToolTip(tr("Show progress detail"));

  FQ_VERIFY(connect(b1, SIGNAL(clicked()), SLOT(abortAllProgressOperations())));
  FQ_VERIFY(connect(b2, SIGNAL(toggled(bool)), SLOT(toggleProgressWindow(bool))));

  popupProgressMessage_ = new PopupMessage(this, mainProgressBarBox, 0);
  popupProgressMessage_->showCloseButton(false);
  popupProgressMessage_->showCounter(false);

  popupProgressMessage_->setFrameStyle(QFrame::Box | QFrame::Raised);
  popupProgressMessage_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void StatusBar::addWidget(QWidget *widget) {
  otherWidgetLayout_->addWidget(widget);
}


/// reimplemented functions

void StatusBar::ensurePolished() {
  QWidget::ensurePolished();

  int h = 0;
  QObjectList list = children();

  //for( QObject * o = list.first(); o; o = list.next() ) {
  foreach(QObject *o, list) {
    if (o->inherits("QWidget")) {
      int _h = static_cast < QWidget * > (o)->minimumSizeHint().height();
      if (_h > h) {
        h = _h;
      }


      if (o->inherits("QLabel")) {
        static_cast < QLabel * > (o)->setIndent(4);
      }
    }
  }

  h -= 4; // it's too big usually

  //for ( QObject * o = list.first(); o; o = list.next() )
  foreach(QObject *o, list)static_cast < QWidget * > (o)->setFixedHeight(h);

  //delete list;
}

void StatusBar::paintEvent(QPaintEvent*) {
  QObjectList list = children(); //queryList( "QWidget", 0, false, false );
  QPainter p(this);

  //for( QObject * o = list.first(); o; o = list.next() ) {
  foreach(QObject *o, list) {
    if (o->inherits("QWidget")) {
      QWidget *w = (QWidget*)o;

      if (!w->isVisible()) {
        continue;
      }

      /*style().drawPrimitive(
        QStyle::PE_StatusBarSection,
        &p,
        QRect( w->x() - 1, w->y() - 1, w->width() + 2, w->height() + 2 ),
        colorGroup(),
        QStyle::State_None,
        QStyleOption( w ) );*/
    }
  }

  //delete list;
}

bool StatusBar::event(QEvent *e) {
  if (e->type() == QEvent::LayoutRequest) {
    update();
  }

  return QWidget::event(e);
}


/// Messaging system

void StatusBar::setMainText(const QString &text) {
  mainText_ = text;

  // it may not be appropriate for us to set the mainText yet
  resetMainText();
}

void StatusBar::shortMessage(const QString &text) {
  m_mainTextLabel->setText(text);
  m_mainTextLabel->setPalette(QToolTip::palette());

  //SingleShotPool::startTimer( 5000, this, SLOT(resetMainText()) );
  resetMainText();
}

void StatusBar::resetMainText() {
  /*
    if( sender() )
    debug() << sender()->name() << endl;

    // don't reset if we are showing a shortMessage
    if( SingleShotPool::isActive( this, SLOT(resetMainText()) ) )
    return;
  */
  m_mainTextLabel->setPalette(QPalette());

  if (allDone()) {
    m_mainTextLabel->setText(mainText_);
  }

  else {
    //m_mainTextLabel->setPaletteBackgroundColor( m_mainTextLabel->paletteBackgroundColor().dark( 110 ) );
    m_mainTextLabel->setPalette(QToolTip::palette());

    ProgressBar *bar = 0;
    uint count = 0;

    for (ProgressMap::ConstIterator it = progressMap_.begin();
        it != progressMap_.end(); ++it ) {
      if (!(*it)->isFinished_) {
        bar =  *it;
        count++;
      }
    }

    if (count == 1) {
      m_mainTextLabel->setText(bar->description() + "...");
    } else {
      m_mainTextLabel->setText(tr("Multiple background-tasks running"));
    }
  }
}
/*
  void
  StatusBar::shortLongMessage( const QString &_short, const QString &_long )
  {
  shortMessage( _short );

  if ( !_long.isEmpty() ) {
  AMAROK_NOTIMPLEMENTED
  }
  }
*/
//void
//StatusBar::longMessage( const QString &text, int /*type*/ )
/*{
  PopupMessage * message;
  message = new PopupMessage( this, m_mainTextLabel );
  message->setText( text );

  if ( !m_messageQueue.isEmpty() )
  message->stackUnder( m_messageQueue.last() );

  message->reposition();
  message->display();

  raise();

  m_messageQueue += message;
  }
*/

//void
//StatusBar::longMessageThreadSafe( const QString &text, int /*type*/ )
/*{
  QCustomEvent * e = new QCustomEvent( 1000 );
  e->setData( new QString( text ) );
  QApplication::postEvent( this, e );
  }
*/
// void
// StatusBar::customEvent( QCustomEvent *e )
// {
//    QString *s = static_cast<QString*>( e->data() );
//    shortMessage( *s );
//    delete s;
//}


/// application wide progress monitor

inline bool StatusBar::allDone() {
  for (ProgressMap::Iterator it = progressMap_.begin(), end =
                                  progressMap_.end(); it != end; ++it)
    if ((*it)->isFinished_ == false) {
      return false;
    }

  return true;
}

ProgressBar &StatusBar::newProgressOperation(QObject *owner) {
  if (progressMap_.contains(owner)) {
    return  *progressMap_[owner];
  }

  if (allDone()) {
    // if we're allDone then we need to remove the old progressBars before
    // we start anything new or the total progress will not be accurate
    pruneProgressBars();
  } else {
    (progressBox()->findChild < QWidget * > ("showAllProgressDetails"))->show();
  }

  QWidget *hbox = new QWidget(popupProgressMessage_);
  QHBoxLayout *hlayout = new QHBoxLayout(hbox);
  QLabel *label = new QLabel(hbox);
  hlayout->addWidget(label);
  ProgressBar *pBar = new ProgressBar(hbox, label);
  hlayout->addWidget(pBar);
  hbox->show();
  popupProgressMessage_->addWidget(hbox);
  progressMap_.insert(owner, pBar);


  FQ_VERIFY(connect(owner, SIGNAL(destroyed(QObject*)), SLOT(endProgressOperation(QObject*))));

  // so we can show the correct progress information
  // after the ProgressBar is setup
  //SingleShotPool::startTimer( 0, this, SLOT(updateProgressAppearance()) );
  updateProgressAppearance();

  progressBox()->show();
  cancelButton()->setEnabled(true);

  return  *progressMap_[owner];
}
/*
  ProgressBar&
  StatusBar::newProgressOperation( KIO::Job *job )
  {
  ProgressBar & bar = newProgressOperation( (QObject*)job );
  bar.setTotalSteps( 100 );

  if(!allDone())
  static_cast<QWidget*>(progressBox()->child("showAllProgressDetails"))->show();
  QVERIFY(connect( job, SIGNAL(result( KIO::Job* )), SLOT(endProgressOperation())));
  //TODO QVERIFY(connect( job, SIGNAL(infoMessage( KIO::Job *job, const QString& )), SLOT() ));
  QVERIFY(connect( job, SIGNAL(percent( KIO::Job*, unsigned long )), SLOT(setProgress( KIO::Job*, unsigned long ))));

  return bar;
  }
*/
/*
  void
  StatusBar::endProgressOperation()
  {
  QObject *owner = (QObject*)sender(); //HACK deconsting it
  KIO::Job *job = dynamic_cast<KIO::Job*>( owner );

  //FIXME doesn't seem to work for KIO::DeleteJob, it has it's own error handler and returns no error too
  // if you try to delete http urls for instance <- KDE SUCKS!

  if( job && job->error() )
  longMessage( job->errorString(), Error );

  endProgressOperation( owner );
  }
*/
void StatusBar::endProgressOperation(QObject *owner) {
  //the owner of this progress operation has been deleted
  //we need to stop listening for progress from it
  //NOTE we don't delete it yet, as this upsets some
  //things, we just call setDone().

  if (!progressMap_.contains(owner)) {
    return ;
  }

  progressMap_[owner]->setDone();

  if (allDone() && popupProgressMessage_->isHidden()) {
    cancelButton()->setEnabled(false);
    //SingleShotPool::startTimer( 2000, this, SLOT(hideMainProgressBar()) );
    hideMainProgressBar();
  }

  updateTotalProgress();
}

void StatusBar::abortAllProgressOperations() { //slot
  for (ProgressMap::Iterator it = progressMap_.begin(), end =
                                  progressMap_.end(); it != end; ++it) {
    (*it)->abortButton_->animateClick();
  }

  m_mainTextLabel->setText(tr("Aborting all jobs..."));

  cancelButton()->setEnabled(false);
}

void StatusBar::toggleProgressWindow(bool show) { //slot
  popupProgressMessage_->adjustSize();
  //FIXME shouldn't be needed, adding bars doesn't seem to do this
  popupProgressMessage_->setVisible(show);

  if (!show) {
    hideMainProgressBar();
  }
  //SingleShotPool::startTimer( 2000, this, SLOT(hideMainProgressBar()) );
}

void StatusBar::showMainProgressBar() {
  if (!allDone()) {
    progressBox()->show();
  }
}

void StatusBar::hideMainProgressBar() {
  if (allDone() && popupProgressMessage_->isHidden()) {
    pruneProgressBars();

    resetMainText();

    mainProgressBar_->setValue(0);
    progressBox()->close();
  }
}

void StatusBar::setProgress(int steps) {
  setProgress(sender(), steps);
}
/*
  void
  StatusBar::setProgress( KIO::Job *job, unsigned long percent )
  {
  setProgress( ( QObject* ) job, percent );
  }
*/
void StatusBar::setProgress(const QObject *owner, int steps) {
  if (!progressMap_.contains(owner)) {
    return ;
  }

  progressMap_[owner]->setValue(steps);

  updateTotalProgress();
}

void StatusBar::setProgressStatus(const QObject *owner, const QString &text) {
  if (!progressMap_.contains(owner)) {
    return ;
  }

  progressMap_[owner]->setStatus(text);
}

void StatusBar::incrementProgress() {
  incrementProgress(sender());
}

void StatusBar::incrementProgress(const QObject *owner) {
  if (!progressMap_.contains(owner)) {
    return ;
  }

  progressMap_[owner]->setValue(progressMap_[owner]->value() + 1);

  updateTotalProgress();
}

void StatusBar::updateTotalProgress() {
  uint totalSteps = 0;
  uint progress = 0;

  for (ProgressMap::ConstIterator it = progressMap_.begin();
       it != progressMap_.end(); ++it ) {
    totalSteps += (*it)->maximum();
    progress += (*it)->value();
  }

  if (totalSteps == 0 && progress == 0) {
    return ;
  }

  mainProgressBar_->setMaximum(totalSteps);
  mainProgressBar_->setValue(progress);

  pruneProgressBars();
}

void StatusBar::updateProgressAppearance() {
  toggleProgressWindowButton()->setVisible(progressMap_.count() > 1);

  resetMainText();

  updateTotalProgress();
}

void StatusBar::pruneProgressBars() {
  ProgressMap::Iterator it = progressMap_.begin();
  const ProgressMap::Iterator end = progressMap_.end();
  int count = 0;
  bool removedBar = false;
  while (it != end)
    if ((*it)->isFinished_ == true) {
      delete (*it)->label_;
      delete (*it)->abortButton_;
      delete (*it);

      ProgressMap::Iterator jt = it;
      ++it;
      progressMap_.erase(jt);
      removedBar = true;
    } else {
      ++it;
      ++count;
    }
  if (count == 1 && removedBar) { //if its gone from 2 or more bars to one bar...
    resetMainText();
    (progressBox()->findChild < QWidget * > ("showAllProgressDetails"))->hide();
    popupProgressMessage_->setVisible(false);
  }
}

} //namespace FQTerm

#include "statusBar.moc"
