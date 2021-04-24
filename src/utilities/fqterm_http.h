// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_HTTP_H
#define FQTERM_HTTP_H

#include <QObject>
#include <QMutex>
#include <QMap>
#include <QScopedPointer>
#include <QNetworkReply>

class QNetworkProxy;
class QNetworkAccessManager;

namespace FQTerm {

class FQTermConfig;
class FQTermFileDialog;

class FQTermHttp: public QObject {
  Q_OBJECT;
 public:
  FQTermHttp(FQTermConfig *, QWidget*, const QString &poolDir, int serverEncodingID);
  ~FQTermHttp();

  void getLink(const QString &, bool);
  void getLink(const QUrl &, bool);
  void setProxy (const QNetworkProxy & proxy);
 signals:
  void done(QObject*);
  void message(const QString &);
  void percent(int);
  void headerReceived(FQTermHttp *, const QString &filename);
  void previewImage(const QString &filename, bool raiseViewer, bool done);

 public slots:
  void cancel();

 protected slots:
   void httpDone();
   void httpRead(qint64, qint64);
   void httpError(QNetworkReply::NetworkError);
   void httpResponse();

 protected:
   QScopedPointer<QNetworkAccessManager> nam_;
   QScopedPointer<QNetworkReply> netreply_;
  QString cacheFileName_;
  bool previewEmitted;
  bool isPreview_;
  bool isExisting_;
  int lastPercent_;
  int serverEncodingID_;

  QString poolDir_;

  static QMap<QString, int> downloadMap_;
  static QMutex mutex_;

  FQTermConfig *config_;
};

}  // namespace FQTerm

#endif  // FQTERM_HTTP_H
