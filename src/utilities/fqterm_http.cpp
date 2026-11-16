// SPDX-License-Identifier: GPL-2.0-or-later

#include "fqterm_path.h"
#include "fqterm_http.h"
#include "fqterm_config.h"
#include "fqterm_filedialog.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QImageReader>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTextCodec>
#include <QTimer>
#include <QUrlQuery>

namespace FQTerm {
namespace {

QString safeFileName(QString name) {
  // Neither response headers nor URLs may select a directory on disk.
  name.replace('\\', '/');
  name = name.section('/', -1);
  name.replace(QRegularExpression("[\\x00-\\x1f<>:\"/\\\\|?*]"), "_");
  name = name.trimmed();
  while (name.endsWith('.') || name.endsWith(' '))
    name.chop(1);
  return name.isEmpty() ? QString("download") : name.left(160);
}

QString responseFileName(const QByteArray &header, const QUrl &url) {
  // Match whole parameters, including quoted semicolons and escaped quotes.
  QRegularExpression parameter(
      "(?:^|;)\\s*(filename\\*?)\\s*=\\s*(?:\"((?:\\\\.|[^\"\\\\])*)\"|([^;]*))",
      QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatchIterator matches = parameter.globalMatch(QString::fromLatin1(header));
  QString name, extendedName;
  while (matches.hasNext()) {
    const QRegularExpressionMatch match = matches.next();
    QString value = match.captured(2).isNull() ? match.captured(3).trimmed() : match.captured(2);
    if (match.captured(1).endsWith('*')) {
      const int first = value.indexOf('\'');
      const int second = value.indexOf('\'', first + 1);
      QTextCodec *codec = first > 0 ? QTextCodec::codecForName(value.left(first).toLatin1()) : nullptr;
      if (second > first && codec)
        extendedName = codec->toUnicode(QByteArray::fromPercentEncoding(value.mid(second + 1).toLatin1()));
    } else {
      // Decode quoted-pair escapes without stripping Windows path separators.
      value.replace("\\\"", "\"");
      name = value;
    }
  }
  if (!extendedName.isEmpty())
    return safeFileName(extendedName);
  if (!name.isEmpty())
    return safeFileName(name);

  name = QFileInfo(url.path()).fileName();
  // Attachment endpoints often put the useful filename in the query only.
  const auto items = QUrlQuery(url).queryItems(QUrl::FullyDecoded);
  const auto formats = QImageReader::supportedImageFormats();
  for (const auto &item : items) {
    const QString candidate = item.second.isEmpty() ? item.first : item.second;
    if (formats.contains(QFileInfo(candidate).suffix().toLower().toLatin1()))
      return safeFileName(candidate);
  }
  return safeFileName(name);
}

QString diagnosticUrl(const QUrl &url) {
  // Query values and userinfo may contain session tokens or credentials.
  QUrl safe = url.adjusted(QUrl::RemoveUserInfo | QUrl::RemoveFragment | QUrl::RemoveQuery);
  return safe.toString() + (url.hasQuery() ? QString("?<redacted>") : QString());
}

}  // namespace

FQTermHttp::FQTermHttp(FQTermConfig *config, QWidget *, const QString &poolDir, int)
    : nam_(new QNetworkAccessManager), poolDir_(poolDir), config_(config) {
}

FQTermHttp::~FQTermHttp() {
}

void FQTermHttp::cancel() {
  if (completed_)
    return;
  canceled_ = true;
  if (netreply_ && netreply_->isRunning())
    netreply_->abort();
  else
    httpDone();
}

void FQTermHttp::getLink(const QString &url, bool preview) {
  getLink(QUrl(url), preview);
}

void FQTermHttp::getLink(const QUrl &url, bool preview) {
  if (netreply_) {
    netreply_->disconnect(this);
    netreply_->abort();
    netreply_.reset();
  }
  download_.reset();
  cacheFileName_.clear();
  suggestedName_.clear();
  failureReason_.clear();
  originalUrl_ = url;
  isPreview_ = preview;
  completed_ = canceled_ = false;

  if (url.isRelative() || url.isLocalFile()) {
    cacheFileName_ = url.isLocalFile() ? url.toLocalFile() : url.toString();
    completed_ = true;
    QImageReader reader(cacheFileName_);
    reader.setDecideFormatFromContent(true);
    if (preview && !reader.read().isNull())
      emit previewImage(cacheFileName_, false, true);
    else
      emit error(tr("Cannot read image: %1").arg(cacheFileName_));
    emit done(this);
    return;
  }

  QUrl target = url;
  if (QFile::exists(getPath(USER_CONFIG) + "hosts.cfg")) {
    FQTermConfig hosts(getPath(USER_CONFIG) + "hosts.cfg");
    const QString host = hosts.getItemValue("hosts", url.host().toLocal8Bit());
    if (!host.isEmpty())
      target.setHost(host);
  }
  QNetworkRequest request(target);
  // Qt resolves relative redirects, limits loops, and rejects HTTPS downgrades.
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                       QNetworkRequest::NoLessSafeRedirectPolicy);
  request.setMaximumRedirectsAllowed(10);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  request.setTransferTimeout(30000);
#endif
  netreply_.reset(nam_->get(request));
  connect(netreply_.data(), &QNetworkReply::finished, this, &FQTermHttp::httpDone);
  connect(netreply_.data(), &QNetworkReply::readyRead, this, &FQTermHttp::httpRead);
  connect(netreply_.data(), &QNetworkReply::downloadProgress, this, &FQTermHttp::httpProgress);
  connect(netreply_.data(), &QNetworkReply::metaDataChanged, this, &FQTermHttp::httpResponse);
}

void FQTermHttp::httpResponse() {
  const int status = netreply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (!completed_ && !choosingFile_ && status >= 200 && status < 300)
    prepareDownload();
}

bool FQTermHttp::prepareDownload() {
  if (download_)
    return true;
  if (canceled_ || !failureReason_.isEmpty())
    return false;
  suggestedName_ = responseFileName(netreply_->rawHeader("Content-Disposition"), netreply_->url());
  QString directory = poolDir_;
  if (!isPreview_) {
    choosingFile_ = true;
    FQTermFileDialog dialog(config_);
    cacheFileName_ = dialog.getSaveName(suggestedName_, "*");
    choosingFile_ = false;
    // The modal dialog runs a nested event loop: finished may arrive there.
    QTimer::singleShot(0, this, [this]() {
      if (!completed_ && netreply_ && netreply_->isFinished())
        httpDone();
    });
    if (canceled_ || cacheFileName_.isEmpty()) {
      cancel();
      return false;
    }
    directory = QFileInfo(cacheFileName_).absolutePath();
  }
  if (!QDir().mkpath(directory)) {
    fail(tr("Cannot create download directory: %1").arg(directory));
    return false;
  }
  download_.reset(new QTemporaryFile(QDir(directory).filePath(".fqterm-download-XXXXXX")));
  if (!download_->open()) {
    fail(tr("Cannot create download file: %1").arg(download_->errorString()));
    return false;
  }
  emit headerReceived(this, suggestedName_);
  return true;
}

void FQTermHttp::httpRead() {
  if (completed_ || canceled_ || choosingFile_ || !failureReason_.isEmpty())
    return;
  const int status = netreply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (status < 200 || status >= 300 || !prepareDownload())
    return;
  const QByteArray data = netreply_->readAll();
  if (download_->write(data) != data.size())
    fail(tr("Cannot write download file: %1").arg(download_->errorString()));
}

void FQTermHttp::httpProgress(qint64 received, qint64 total) {
  if (!completed_ && !canceled_ && total > 0)
    emit percent(qBound(0, int(100.0 * received / total), 100));
}

void FQTermHttp::fail(const QString &reason) {
  failureReason_ = reason;
  if (netreply_ && netreply_->isRunning())
    netreply_->abort();
}

void FQTermHttp::reportResult(const QString &reason) {
  if (netreply_) {
    QString networkError = netreply_->errorString();
    networkError.replace(netreply_->url().toString(), diagnosticUrl(netreply_->url()));
    networkError.replace(originalUrl_.toString(), diagnosticUrl(originalUrl_));
    qInfo().noquote() << "FQTerm download: url=" << diagnosticUrl(netreply_->url())
        << "status=" << netreply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
        << "networkError=" << int(netreply_->error()) << networkError
        << "contentType=" << netreply_->rawHeader("Content-Type")
        << "contentDisposition=" << netreply_->rawHeader("Content-Disposition").left(512)
        << "cache=" << cacheFileName_
        << "bytes=" << (download_ ? download_->size() : 0)
        << "result=" << reason;
  }
}

void FQTermHttp::httpDone() {
  if (completed_ || choosingFile_)
    return;
  if (netreply_ && !canceled_ && failureReason_.isEmpty() &&
      netreply_->error() == QNetworkReply::NoError)
    httpRead();  // finished may deliver bytes without a final progress update.
  if (completed_)
    return;
  completed_ = true;

  QString reason = failureReason_;
  if (!canceled_ && reason.isEmpty() && netreply_) {
    const int status = netreply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (netreply_->error() != QNetworkReply::NoError || status < 200 || status >= 300) {
      QString detail = netreply_->errorString();
      if (netreply_->error() == QNetworkReply::OperationCanceledError)
        detail = tr("Download timed out: no data was received for 30 seconds.");
      detail.replace(netreply_->url().toString(), diagnosticUrl(netreply_->url()));
      detail.replace(originalUrl_.toString(), diagnosticUrl(originalUrl_));
      reason = tr("Download failed (HTTP %1, Qt error %2): %3")
          .arg(status).arg(int(netreply_->error())).arg(detail);
    }
  }
  if (!canceled_ && reason.isEmpty() && (!download_ || !download_->flush()))
    reason = tr("The download is empty or could not be saved.");

  if (!canceled_ && reason.isEmpty() && isPreview_) {
    QImageReader reader(download_->fileName());
    reader.setDecideFormatFromContent(true);
    const QByteArray format = reader.format().toLower();
    if (reader.read().isNull()) {
      reason = tr("The downloaded file is not a readable image (Content-Type: %1): %2")
          .arg(netreply_->header(QNetworkRequest::ContentTypeHeader).toString(), reader.errorString());
    } else {
      // Key by the ORIGINAL complete URL, including all query parameters.
      const QByteArray key = QCryptographicHash::hash(
          originalUrl_.adjusted(QUrl::RemoveFragment).toEncoded(), QCryptographicHash::Sha256).toHex();
      cacheFileName_ = QDir(poolDir_).filePath(QString("%1-%2.%3")
          .arg(QFileInfo(suggestedName_).completeBaseName().left(80),
               QString::fromLatin1(key), QString::fromLatin1(format)));
    }
  }

  if (!canceled_ && reason.isEmpty()) {
    // Atomic replacement prevents partial files and same-size cache mixups.
    QSaveFile output(cacheFileName_);
    if (!download_->seek(0) || !output.open(QIODevice::WriteOnly)) {
      reason = tr("Cannot save download: %1").arg(output.errorString());
    } else {
      while (!download_->atEnd()) {
        const QByteArray block = download_->read(65536);
        if (block.isEmpty() || output.write(block) != block.size()) {
          reason = tr("Cannot save download: %1").arg(output.errorString());
          break;
        }
      }
      if (reason.isEmpty() && !output.commit())
        reason = tr("Cannot save download: %1").arg(output.errorString());
    }
  }
  reportResult(canceled_ ? QString("canceled") : reason.isEmpty() ? QString("success") : reason);
  download_.reset();
  if (!canceled_) {
    if (!reason.isEmpty())
      emit error(reason);
    else if (isPreview_)
      emit previewImage(cacheFileName_, true, true);
    else
      emit message(tr("Download one file successfully"));
  }
  emit done(this);
}

void FQTermHttp::setProxy(const QNetworkProxy &proxy) {
  nam_->setProxy(proxy);
}

}  // namespace FQTerm
