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

#include "fqterm.h"
#include "fqterm_path.h"
#include "fqterm_http.h"
#include "fqterm_config.h"
#include "fqterm_filedialog.h"

namespace FQTerm {

QMap<QString, int> FQTermHttp::downloadMap_;
QMutex FQTermHttp::mutex_;

FQTermHttp::FQTermHttp(FQTermConfig *config, QWidget *p, const QString &poolDir, int serverEncodingID)
  : nam_(new QNetworkAccessManager), poolDir_(poolDir)  {
  // 	m_pDialog = NULL;
  config_ = config;

  serverEncodingID_ = serverEncodingID;

 }

FQTermHttp::~FQTermHttp() {
}

void FQTermHttp::cancel() {
  if(netreply_){
    netreply_->abort();
  }
  if (QFile::exists(cacheFileName_)) {
    QFile::remove(cacheFileName_);
  }

  emit done(this);
}



void FQTermHttp::getLink(const QString &url, bool preview) {
  QUrl u(url);
  getLink(u,preview);
}

void FQTermHttp::getLink(const QUrl& url, bool preview) {
  QUrl u=url;
  isExisting_ = false;
  isPreview_ = preview;
  previewEmitted = false;
  lastPercent_ = 0;
  if (u.isRelative() || u.scheme() == "file") {
    emit previewImage(cacheFileName_, false, true);
    emit done(this);
    return ;
  }

  if (QFile::exists(getPath(USER_CONFIG) + "hosts.cfg")) {
    config_ = new FQTermConfig(getPath(USER_CONFIG) + "hosts.cfg");
    QString strTmp = config_->getItemValue("hosts", u.host().toLocal8Bit());
    if (!strTmp.isEmpty()) {
      QString strUrl = u.toString();
      strUrl.replace(QRegExp(u.host(), Qt::CaseInsensitive), strTmp);
      u = strUrl;
    }
  }
  if (!(netreply_ && netreply_->hasRawHeader("Location"))) {
    cacheFileName_ = QFileInfo(u.path()).fileName();
  }
  if(netreply_){
    netreply_->blockSignals(true);
    netreply_.take()->deleteLater();
  }

  netreply_.reset(nam_->get(QNetworkRequest(u)));
  FQ_VERIFY(connect(netreply_.data(), SIGNAL(finished()), this, SLOT(httpDone())));
  FQ_VERIFY(connect(netreply_.data(), SIGNAL(downloadProgress(qint64, qint64)),this, SLOT(httpRead(qint64, qint64))));
  FQ_VERIFY(connect(netreply_.data(), SIGNAL(error( QNetworkReply::NetworkError)), this, SLOT(httpError(QNetworkReply::NetworkError))));
  FQ_VERIFY(connect(netreply_.data(), SIGNAL(metaDataChanged()), this, SLOT(httpResponse())));
}

void FQTermHttp::httpResponse() {
  if (netreply_->hasRawHeader("Location")) {
    // use rawHeader("Location") instead of header(QNetworkRequest::LocationHeader)
    // it works for both absolute and relative redirection
    QUrl u(netreply_->rawHeader("Location"));
    if (u.isRelative() ) {
      u=netreply_->url().resolved(u);
    }
    cacheFileName_ = QFileInfo(u.path()).fileName(); // update filename
    getLink(u,isPreview_);
    return;
  }

  QString ValueString;
  QString filename;

  ValueString = netreply_->header(QNetworkRequest::ContentLengthHeader).toString();
  int FileLength = ValueString.toInt();

  ValueString = netreply_->rawHeader("Content-Disposition");
  //	ValueString = ValueString.mid(ValueString.find(';') + 1).stripWhiteSpace();
  //	if(ValueString.lower().find("filename") == 0)
  //	m_strHttpFile = ValueString.mid(ValueString.find('=') + 1).stripWhiteSpace();
  if (ValueString.right(1) != ";") {
    ValueString += ";";
  }
  QRegExp re("filename=.*;", Qt::CaseInsensitive);
  re.setMinimal(true);
  //Dont FIXME:this will also split filenames with ';' inside, does anyone really do this?
  int pos = re.indexIn(ValueString);
  if (pos != -1) {
    cacheFileName_ = ValueString.mid(pos + 9, re.matchedLength() - 10);
  }
  //cacheFileName_ = encoding2unicode(cacheFileName_.toLatin1(), serverEncodingID_);
  filename = cacheFileName_;

  if (isPreview_) {
    cacheFileName_ = poolDir_ + cacheFileName_;

    QFileInfo fi(cacheFileName_);

    int i = 1;
    QFileInfo fi2 = fi;

    mutex_.lock();
    if (downloadMap_.find(cacheFileName_) == downloadMap_.end() && !fi2.exists()) { 
      downloadMap_[cacheFileName_] = FileLength;
    }
    while (fi2.exists()) {

      QMap<QString, int>::iterator ii;
      if ((ii = downloadMap_.find(cacheFileName_)) != downloadMap_.end()) { 
        if (ii.value() == FileLength) {
          mutex_.unlock();
          netreply_->abort();
          isExisting_ = true;
          emit headerReceived(this, filename);
          emit done(this);
          return;
        }
      }

      if (fi2.size() == FileLength) {
        mutex_.unlock();
        isExisting_ = true;
        emit headerReceived(this, filename);
        netreply_->abort();
        return;
      } else {
		    cacheFileName_ = QString("%1/%2(%3).%4").arg(fi.path())
								.arg(fi.completeBaseName()).arg(i).arg(fi.suffix());
        fi2.setFile(cacheFileName_);
        if (!fi2.exists()) {
          downloadMap_[cacheFileName_] = FileLength;
          break;
        }
        
        i++;
      }
    }
    mutex_.unlock();

    fi.setFile(cacheFileName_);

    QString strExt = fi.suffix().toLower();
    if (strExt == "jpg" || strExt == "jpeg" || strExt == "gif" || strExt == "mng"
        || strExt == "png" || strExt == "bmp") {
      isPreview_ = true;
    } else {
      isPreview_ = false;
    }
  } else {
    //getSaveFileName(cacheFileName_, NULL, strSave);
	  mutex_.lock();

	  FQTermFileDialog fileDialog(config_);
    QString strSave = fileDialog.getSaveName(cacheFileName_, "*");
	  mutex_.unlock();
    // no filename specified which means the user canceled this download
    if (strSave.isEmpty()) {
      netreply_->abort();
      emit done(this);
      return ;
    }
    cacheFileName_ = strSave;
  }

  emit headerReceived(this, filename);
}


void FQTermHttp::httpRead(qint64 done, qint64 total) {
  QByteArray ba = netreply_->readAll();
  QFile file(cacheFileName_);
  if (file.open(QIODevice::ReadWrite | QIODevice::Append)) {
    QDataStream ds(&file);
    ds.writeRawData(ba, ba.size());
    file.close();
  }
  if (total != 0) {
	//m_pDialog->setProgress(done,total);
    int p = done *100 / total;
    if (p - lastPercent_ >= 10 && isPreview_ && QFileInfo(cacheFileName_).suffix().toLower() == "jpg") {
      if (!previewEmitted) {
        emit previewImage(cacheFileName_,true, false);
        previewEmitted = true;
      } else {
        emit previewImage(cacheFileName_,false, false);
      }
      lastPercent_ = p;
    }
    emit percent(p);
  }
}

void FQTermHttp::httpDone() {
    mutex_.lock();
    downloadMap_.remove(cacheFileName_);
    mutex_.unlock();
    if (isPreview_) {
      emit previewImage(cacheFileName_, true, true);
    } else {
      emit message("Download one file successfully");
    }
    emit done(this);
}

void FQTermHttp::httpError(QNetworkReply::NetworkError code) {
    switch(code)
    {
    case QNetworkReply::OperationCanceledError:
      break;
    default:
      QMessageBox::critical(NULL, tr("Download Error"), tr("Failed to download file\n code=%1").arg(code));
      //deleteLater(); /*Not needed. http done will be called and this will cause a double free.
      //return;
      break;
    }
}

void FQTermHttp::setProxy(const QNetworkProxy & proxy) {
  nam_->setProxy(proxy);
}
}  // namespace FQTerm

#include "fqterm_http.moc"
