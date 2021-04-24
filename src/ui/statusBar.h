// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_STATUSBAR_H
#define FQTERM_STATUSBAR_H

#include "progressBar.h" //convenience
// #include <qwidget.h>     //baseclass
// #include <qmap.h>        //stack allocated
// #include <q3valuelist.h>  //stack allocated
//Added by qt3to4:
// #include <QCustomEvent>
#include <QLabel>
#include <QEvent>
#include <QPaintEvent>
#include <QMap>
#include <QList>
#include <QWidget>
#include <QProgressBar>

class QString;
class QLayout;
class QLabel;
class QString;

namespace FQTerm {

class PopupMessage;
typedef QMap < const QObject *, ProgressBar * > ProgressMap;

class StatusBar: public QWidget {
  Q_OBJECT;
 public:
  StatusBar(QWidget *parent_, const char *name = "mainStatusBar");
  static StatusBar *instance() {
    return singletonStatusBar_
        ;
  }
  /**
   * Start a progress operation, if owner is 0, the return value is
   * undefined - the application will probably crash.
   * @param owner controls progress for this operation
   * @return the progressBar so you can configure its parameters
   * @see setProgress( QObject*, int )
   * @see incrementProgress( QObject* )
   * @see setProgressStatus( const QObject*, const QString& )
   */
  ProgressBar &newProgressOperation(QObject *owner);

  void setProgress(const QObject *owner, int steps);
  void incrementProgress(const QObject *owner);
  void setProgressStatus(const QObject *owner, const QString &text);
 public slots:
  void setMainText(const QString &text);
  void resetMainText();
  void shortMessage(const QString &text);
  /** Stop anticipating progress from sender() */
  //void endProgressOperation();

  /** Stop anticipating progress from @param owner */
  void endProgressOperation(QObject *owner);

  /**
   * Convenience function works like setProgress( QObject*, int )
   * Uses the return value from sender() to determine the owner of
   * the progress bar in question
   */
  void setProgress(int steps);

  /**
   * Convenience function works like incrementProgress( QObject* )
   * Uses the return value from sender() to determine the owner of
   * the progress bar in question
   */
  void incrementProgress();

 public slots:
  void toggleProgressWindow(bool show);
  void abortAllProgressOperations();

 private slots:
  /** For internal use against KIO::Jobs */
  //void setProgress( KIO::Job*, unsigned long percent );
  void showMainProgressBar();
  void hideMainProgressBar();
  void updateProgressAppearance();

 protected:
  virtual void ensurePolished();
  // 		virtual void customEvent( QCustomEvent* );
  virtual void paintEvent(QPaintEvent*);
  virtual bool event(QEvent*);

  /**
   * You must parent the widget to the statusbar, we won't do that
   * for you! The widget will be added to the right of the layout.
   * Currently you must add widgets before the statusbar gets shown
   * for the first time, because we are not currently very flexible.
   */
  void addWidget(QWidget *widget);

  QLabel *m_mainTextLabel;
 private:
  void updateTotalProgress();
  bool allDone(); ///@return true if all progress operations are complete
  void pruneProgressBars(); /// deletes old progress bars

  QWidget *cancelButton() {
    return findChild<QWidget *>("cancelButton");
  }
  QWidget *toggleProgressWindowButton() {
    return findChild<QWidget *>("showAllProgressDetails");
  }
  QWidget *progressBox() {
    return findChild <QWidget *>("progressBox");
  }

  PopupMessage *popupProgressMessage_;
  QProgressBar *mainProgressBar_;

  ProgressMap progressMap_;
  //  QList<QWidget *> m_messageQueue;
  QString mainText_;

  QLayout *otherWidgetLayout_;
  static StatusBar *singletonStatusBar_;
};

}  // namespace FQTerm

#endif  // FQTERM_STATUSBAR_H
