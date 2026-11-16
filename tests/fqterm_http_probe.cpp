// SPDX-License-Identifier: GPL-2.0-or-later

#include "fqterm_http.h"
#include <QApplication>
#include <QDebug>
#include <QImage>
#include <QNetworkProxy>
#include <QSslSocket>
#include <QTimer>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  if (app.arguments().size() != 3) {
    qCritical() << "Usage: fqterm_http_probe URL CACHE_DIRECTORY";
    return 2;
  }
  qInfo() << "SSL supported:" << QSslSocket::supportsSsl()
          << "built:" << QSslSocket::sslLibraryBuildVersionString()
          << "runtime:" << QSslSocket::sslLibraryVersionString();
  FQTerm::FQTermHttp http(nullptr, nullptr, app.arguments().at(2), 0);
  http.setProxy(QNetworkProxy::NoProxy);
  bool previewed = false;
  QObject::connect(&http, &FQTerm::FQTermHttp::error, &app,
                   [](const QString &error) { qCritical().noquote() << error; });
  QObject::connect(&http, &FQTerm::FQTermHttp::previewImage, &app,
                   [&](const QString &file, bool, bool) {
    QImage image(file);
    previewed = !image.isNull();
    qInfo() << "Preview:" << file << "size:" << image.size();
  });
  QObject::connect(&http, &FQTerm::FQTermHttp::done, &app,
                   [&]() { app.exit(previewed ? 0 : 1); });
  QTimer::singleShot(45000, &app, [&]() { http.cancel(); app.exit(3); });
  QTimer::singleShot(0, &app, [&]() { http.getLink(QUrl(app.arguments().at(1)), true); });
  return app.exec();
}
