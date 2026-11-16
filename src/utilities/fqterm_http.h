// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_HTTP_H
#define FQTERM_HTTP_H

#include <QObject>
#include <QScopedPointer>
#include <QNetworkReply>
#include <QTemporaryFile>

class QNetworkProxy;
class QNetworkAccessManager;
class QWidget;

namespace FQTerm {

class FQTermConfig;

class FQTermHttp: public QObject {
  Q_OBJECT
 public:
  FQTermHttp(FQTermConfig *, QWidget *, const QString &poolDir, int serverEncodingID);
  ~FQTermHttp();

  void getLink(const QString &, bool);
  void getLink(const QUrl &, bool);
  void setProxy(const QNetworkProxy &proxy);

 signals:
  void done(QObject *);
  void message(const QString &);
  void error(const QString &);
  void percent(int);
  void headerReceived(FQTermHttp *, const QString &filename);
  void previewImage(const QString &filename, bool raiseViewer, bool done);

 public slots:
  void cancel();

 protected slots:
  void httpDone();
  void httpRead();
  void httpProgress(qint64, qint64);
  void httpResponse();

 protected:
  QScopedPointer<QNetworkAccessManager> nam_;

 private:
  void fail(const QString &reason);
  void reportResult(const QString &reason);
  bool prepareDownload();

  QScopedPointer<QNetworkReply> netreply_;
  QScopedPointer<QTemporaryFile> download_;
  QUrl originalUrl_;
  QString cacheFileName_;
  QString suggestedName_;
  QString failureReason_;
  bool isPreview_ = false;
  bool completed_ = false;
  bool canceled_ = false;
  bool choosingFile_ = false;
  QString poolDir_;
  FQTermConfig *config_;
};

}  // namespace FQTerm

#endif  // FQTERM_HTTP_H
