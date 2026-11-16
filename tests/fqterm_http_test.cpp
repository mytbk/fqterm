// SPDX-License-Identifier: GPL-2.0-or-later

#include "fqterm_http.h"

#include <QBuffer>
#include <QFile>
#include <QFileDialog>
#include <QElapsedTimer>
#include <QImage>
#include <QNetworkProxy>
#include <QSignalSpy>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

using FQTerm::FQTermHttp;

struct Response {
  QByteArray body;
  QByteArray type = "image/jpeg";
  QByteArray disposition;
  QByteArray location;
  int status = 200;
  bool unknownLength = false;
  bool hang = false;
  bool truncated = false;
};

class HttpServer: public QTcpServer {
 public:
  QMap<QByteArray, Response> responses;
  QList<QByteArray> requests;

  HttpServer() {
    connect(this, &QTcpServer::newConnection, this, [this]() {
      while (hasPendingConnections()) {
        QTcpSocket *socket = nextPendingConnection();
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
        connect(socket, &QTcpSocket::readyRead, socket, [this, socket]() {
          QByteArray request = socket->property("request").toByteArray() + socket->readAll();
          socket->setProperty("request", request);
          if (!request.contains("\r\n\r\n") || socket->property("served").toBool())
            return;
          socket->setProperty("served", true);
          const QByteArray path = request.split(' ').value(1);
          requests.append(path);
          const Response response = responses.value(path);
          if (response.hang)
            return;
          QByteArray header = "HTTP/1.1 " + QByteArray::number(response.status) + " Test\r\nConnection: close\r\n";
          header += "Content-Type: " + response.type + "\r\n";
          if (!response.disposition.isEmpty())
            header += "Content-Disposition: " + response.disposition + "\r\n";
          if (!response.location.isEmpty())
            header += "Location: " + response.location + "\r\n";
          if (!response.unknownLength)
            header += "Content-Length: " + QByteArray::number(response.body.size() + (response.truncated ? 100 : 0)) + "\r\n";
          socket->write(header + "\r\n" + response.body);
          socket->disconnectFromHost();
        });
      }
    });
    listen(QHostAddress::LocalHost);
  }

  QUrl url(const QByteArray &path) const {
    return QUrl(QString("http://127.0.0.1:%1%2").arg(serverPort()).arg(QString::fromLatin1(path)));
  }
};

class TlsServer: public QTcpServer {
 protected:
  void incomingConnection(qintptr descriptor) override {
    QSslSocket *socket = new QSslSocket(this);
    socket->setSocketDescriptor(descriptor);
    QFile certificate(QString(FQTERM_TEST_SOURCE_DIR) + "/tls-certificate.pem");
    QFile key(QString(FQTERM_TEST_SOURCE_DIR) + "/tls-key.pem");
    certificate.open(QIODevice::ReadOnly);
    key.open(QIODevice::ReadOnly);
    socket->setLocalCertificate(QSslCertificate(certificate.readAll()));
    socket->setPrivateKey(QSslKey(key.readAll(), QSsl::Rsa));
    connect(socket, &QSslSocket::disconnected, socket, &QObject::deleteLater);
    socket->startServerEncryption();
  }
};

class HttpTest: public QObject {
  Q_OBJECT

  QByteArray image(const char *format = "JPEG", QRgb color = qRgb(230, 40, 30)) {
    QImage picture(24, 32, QImage::Format_RGB32);
    picture.fill(color);
    QByteArray result;
    QBuffer buffer(&result);
    buffer.open(QIODevice::WriteOnly);
    picture.save(&buffer, format);
    return result;
  }

 private slots:
  void initTestCase() {
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
  }

  void images_data() {
    QTest::addColumn<QByteArray>("path");
    QTest::addColumn<QByteArray>("disposition");
    QTest::addColumn<QByteArray>("type");
    QTest::addColumn<QByteArray>("format");
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("unknownLength");
    QTest::newRow("query-only-jpeg") << QByteArray("/att.php?photo.jpg") << QByteArray() << QByteArray("image/jpeg") << QByteArray("JPEG") << QString("photo.jpg") << false;
    QTest::newRow("direct-jpeg") << QByteArray("/photo.jpg?token=abc") << QByteArray() << QByteArray("image/jpeg") << QByteArray("JPEG") << QString("photo.jpg") << false;
    QTest::newRow("query-plus-preserved") << QByteArray("/att.php?opaque+attachment.jpg") << QByteArray() << QByteArray("image/jpeg") << QByteArray("JPEG") << QString("opaque+attachment.jpg") << false;
    QTest::newRow("direct-png") << QByteArray("/photo.png") << QByteArray() << QByteArray("image/png") << QByteArray("PNG") << QString("photo.png") << false;
    QTest::newRow("generic-content-type") << QByteArray("/att.php?id=1") << QByteArray() << QByteArray("application/octet-stream") << QByteArray("JPEG") << QString("att.php") << false;
    QTest::newRow("unknown-length") << QByteArray("/att.php?name=photo.jpg&token=abc") << QByteArray() << QByteArray("image/jpeg") << QByteArray("JPEG") << QString("photo.jpg") << true;
    QTest::newRow("filename") << QByteArray("/att.php") << QByteArray("attachment; filename=photo.jpg") << QByteArray("image/jpeg") << QByteArray("JPEG") << QString("photo.jpg") << false;
    QTest::newRow("quoted-semicolon") << QByteArray("/att.php") << QByteArray("attachment; filename=\"my; photo.jpg\"; size=123") << QByteArray("image/jpeg") << QByteArray("JPEG") << QString("my; photo.jpg") << false;
    QTest::newRow("filename-star") << QByteArray("/att.php") << QByteArray("attachment; filename=fallback.jpg; filename*=UTF-8''%E5%9B%BE%E7%89%87.jpg") << QByteArray("image/jpeg") << QByteArray("JPEG") << QString::fromUtf8("\xe5\x9b\xbe\xe7\x89\x87.jpg") << false;
    QTest::newRow("filename-traversal") << QByteArray("/att.php") << QByteArray("attachment; filename=\"../../photo.jpg\"") << QByteArray("image/jpeg") << QByteArray("JPEG") << QString("photo.jpg") << false;
    QTest::newRow("misleading-extension") << QByteArray("/photo.jpg") << QByteArray() << QByteArray("image/jpeg") << QByteArray("PNG") << QString("photo.jpg") << false;
  }

  void images() {
    QFETCH(QByteArray, path);
    QFETCH(QByteArray, disposition);
    QFETCH(QByteArray, type);
    QFETCH(QByteArray, format);
    QFETCH(QString, name);
    QFETCH(bool, unknownLength);
    HttpServer server;
    QVERIFY(server.isListening());
    Response response;
    response.body = image(format.constData());
    response.type = type;
    response.disposition = disposition;
    response.unknownLength = unknownLength;
    server.responses[path] = response;
    QTemporaryDir pool;
    FQTermHttp http(nullptr, nullptr, pool.path(), 0);
    http.setProxy(QNetworkProxy::NoProxy);
    QSignalSpy done(&http, &FQTermHttp::done), preview(&http, &FQTermHttp::previewImage);
    QSignalSpy error(&http, &FQTermHttp::error), header(&http, &FQTermHttp::headerReceived);
    http.getLink(server.url(path), true);
    QVERIFY(done.wait(5000));
    QCOMPARE(done.count(), 1);
    QCOMPARE(error.count(), 0);
    QCOMPARE(preview.count(), 1);
    QCOMPARE(header.count(), 1);
    QCOMPARE(header.first().at(1).toString(), name);
    QCOMPARE(server.requests.first(), path);
    const QString file = preview.first().at(0).toString();
    QVERIFY(file.startsWith(pool.path() + '/'));
    QVERIFY(file.endsWith(format == "PNG" ? ".png" : ".jpeg"));
    QFile downloaded(file);
    QVERIFY(downloaded.open(QIODevice::ReadOnly));
    QCOMPARE(downloaded.readAll(), response.body);
    QVERIFY(!QImage(file).isNull());
    QCOMPARE(QDir(pool.path()).entryList(QDir::Files | QDir::Hidden).size(), 1);
  }

  void redirects_data() {
    QTest::addColumn<bool>("absolute");
    QTest::newRow("relative") << false;
    QTest::newRow("absolute") << true;
  }

  void redirects() {
    QFETCH(bool, absolute);
    HttpServer server;
    Response redirect, picture;
    redirect.status = 302;
    redirect.location = absolute ? server.url("/att.php?final.jpg").toEncoded() : "att.php?final.jpg";
    redirect.body = "redirect body must not reach the image";
    picture.body = image();
    server.responses["/start"] = redirect;
    server.responses["/att.php?final.jpg"] = picture;
    QTemporaryDir pool;
    FQTermHttp http(nullptr, nullptr, pool.path(), 0);
    QSignalSpy done(&http, &FQTermHttp::done), preview(&http, &FQTermHttp::previewImage);
    http.getLink(server.url("/start"), true);
    QVERIFY(done.wait(5000));
    QCOMPARE(preview.count(), 1);
    QCOMPARE(server.requests.size(), 2);
  }

  void failures_data() {
    QTest::addColumn<int>("status");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("type");
    QTest::newRow("404") << 404 << image() << QByteArray("image/jpeg");
    QTest::newRow("html") << 200 << QByteArray("<html>login required</html>") << QByteArray("text/html");
    QTest::newRow("lying-mime") << 200 << QByteArray("<html>not a JPEG</html>") << QByteArray("image/jpeg");
    QTest::newRow("empty") << 200 << QByteArray() << QByteArray("image/jpeg");
    QTest::newRow("corrupt-jpeg") << 200 << image().left(40) << QByteArray("image/jpeg");
    QTest::newRow("redirect-loop") << 302 << QByteArray() << QByteArray("text/html");
    QTest::newRow("truncated-transfer") << 200 << image() << QByteArray("image/jpeg");
  }

  void failures() {
    QFETCH(int, status);
    QFETCH(QByteArray, body);
    QFETCH(QByteArray, type);
    HttpServer server;
    Response response;
    response.status = status;
    response.body = body;
    response.type = type;
    response.truncated = QByteArray(QTest::currentDataTag()) == "truncated-transfer";
    if (status == 302)
      response.location = "/bad.jpg";
    server.responses["/bad.jpg"] = response;
    QTemporaryDir pool;
    FQTermHttp http(nullptr, nullptr, pool.path(), 0);
    QSignalSpy done(&http, &FQTermHttp::done), preview(&http, &FQTermHttp::previewImage);
    QSignalSpy error(&http, &FQTermHttp::error), message(&http, &FQTermHttp::message);
    http.getLink(server.url("/bad.jpg"), true);
    QVERIFY(done.wait(5000));
    QCOMPARE(done.count(), 1);
    QCOMPARE(error.count(), 1);
    QCOMPARE(preview.count(), 0);
    QCOMPARE(message.count(), 0);
    QVERIFY(QDir(pool.path()).entryList(QDir::Files | QDir::Hidden).isEmpty());
  }

  void concurrentQueryCache() {
    HttpServer server;
    Response first, second;
    first.body = image("PNG", qRgb(250, 0, 0));
    second.body = image("PNG", qRgb(0, 0, 250));
    // Equal byte counts must never make distinct attachments share a file.
    const int length = qMax(first.body.size(), second.body.size());
    first.body.append(QByteArray(length - first.body.size(), '\0'));
    second.body.append(QByteArray(length - second.body.size(), '\0'));
    server.responses["/att.php?id=1"] = first;
    server.responses["/att.php?id=2"] = second;
    QTemporaryDir pool;
    FQTermHttp a(nullptr, nullptr, pool.path(), 0), b(nullptr, nullptr, pool.path(), 0);
    QSignalSpy pa(&a, &FQTermHttp::previewImage), pb(&b, &FQTermHttp::previewImage);
    a.getLink(server.url("/att.php?id=1"), true);
    b.getLink(server.url("/att.php?id=2"), true);
    QTRY_COMPARE(pa.count(), 1);
    QTRY_COMPARE(pb.count(), 1);
    const QString fa = pa.first().at(0).toString(), fb = pb.first().at(0).toString();
    QVERIFY(fa != fb);
    QCOMPARE(QImage(fa).pixel(0, 0), qRgb(250, 0, 0));
    QCOMPARE(QImage(fb).pixel(0, 0), qRgb(0, 0, 250));
    // Re-fetching the same URL replaces the cache atomically without appending.
    a.getLink(server.url("/att.php?id=1"), true);
    QTRY_COMPARE(pa.count(), 2);
    QCOMPARE(pa.last().at(0).toString(), fa);
    QCOMPARE(QFileInfo(fa).size(), qint64(first.body.size()));
  }

  void cancellation() {
    HttpServer server;
    Response response;
    response.hang = true;
    server.responses["/hang.jpg"] = response;
    QTemporaryDir pool;
    FQTermHttp http(nullptr, nullptr, pool.path(), 0);
    QSignalSpy done(&http, &FQTermHttp::done), preview(&http, &FQTermHttp::previewImage);
    QSignalSpy error(&http, &FQTermHttp::error);
    http.getLink(server.url("/hang.jpg"), true);
    QTimer::singleShot(50, &http, &FQTermHttp::cancel);
    QVERIFY(done.wait(5000));
    http.cancel();
    QCOMPARE(done.count(), 1);
    QCOMPARE(error.count(), 0);
    QCOMPARE(preview.count(), 0);
  }

  void unwritablePool() {
    HttpServer server;
    Response response;
    response.body = image();
    server.responses["/image.jpg"] = response;
    QTemporaryDir directory;
    const QString path = directory.filePath("file-not-directory");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    FQTermHttp http(nullptr, nullptr, path, 0);
    QSignalSpy done(&http, &FQTermHttp::done), error(&http, &FQTermHttp::error);
    QSignalSpy preview(&http, &FQTermHttp::previewImage);
    http.getLink(server.url("/image.jpg"), true);
    QVERIFY(done.wait(5000));
    QCOMPARE(error.count(), 1);
    QCOMPARE(preview.count(), 0);
  }

  void localFile() {
    QTemporaryDir pool;
    const QString file = pool.filePath("local.php");
    QFile output(file);
    QVERIFY(output.open(QIODevice::WriteOnly));
    output.write(image());
    output.close();
    FQTermHttp http(nullptr, nullptr, pool.path(), 0);
    QSignalSpy preview(&http, &FQTermHttp::previewImage);
    http.getLink(QUrl::fromLocalFile(file), true);
    QCOMPARE(preview.count(), 1);
    QCOMPARE(preview.first().at(0).toString(), file);
  }

  void saveDownload_data() {
    QTest::addColumn<bool>("cancelSave");
    QTest::newRow("save-file") << false;
    QTest::newRow("cancel-save") << true;
  }

  void saveDownload() {
    QFETCH(bool, cancelSave);
    HttpServer server;
    Response response;
    response.body = "an ordinary downloaded file";
    response.type = "text/plain";
    server.responses["/file.txt"] = response;
    QTemporaryDir pool;
    const QString target = pool.filePath("saved.txt");
    FQTermHttp http(nullptr, nullptr, pool.path(), 0);
    QSignalSpy done(&http, &FQTermHttp::done), preview(&http, &FQTermHttp::previewImage);
    QSignalSpy error(&http, &FQTermHttp::error), message(&http, &FQTermHttp::message);
    QTimer dialogTimer;
    connect(&dialogTimer, &QTimer::timeout, this, [&]() {
      for (QWidget *widget : QApplication::topLevelWidgets()) {
        QFileDialog *dialog = qobject_cast<QFileDialog *>(widget);
        if (dialog && dialog->isVisible()) {
          dialogTimer.stop();
          if (cancelSave)
            dialog->reject();
          else {
            dialog->selectFile(target);
            QMetaObject::invokeMethod(dialog, "accept");
          }
          return;
        }
      }
    });
    dialogTimer.start(100);
    http.getLink(server.url("/file.txt"), false);
    QVERIFY(done.wait(5000));
    QCOMPARE(done.count(), 1);
    QCOMPARE(error.count(), 0);
    QCOMPARE(preview.count(), 0);
    QCOMPARE(message.count(), cancelSave ? 0 : 1);
    if (!cancelSave) {
      QFile saved(target);
      QVERIFY(saved.open(QIODevice::ReadOnly));
      QCOMPARE(saved.readAll(), response.body);
    } else {
      QVERIFY(!QFile::exists(target));
    }
  }

  void tlsCertificateError() {
    QVERIFY2(QSslSocket::supportsSsl(), "Qt SSL runtime must be deployed for HTTPS");
    TlsServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    QTemporaryDir pool;
    FQTermHttp http(nullptr, nullptr, pool.path(), 0);
    QSignalSpy done(&http, &FQTermHttp::done), error(&http, &FQTermHttp::error);
    QSignalSpy preview(&http, &FQTermHttp::previewImage);
    http.getLink(QUrl(QString("https://127.0.0.1:%1/image.jpg").arg(server.serverPort())), true);
    QVERIFY(done.wait(5000));
    QCOMPARE(error.count(), 1);
    QCOMPARE(preview.count(), 0);
    QVERIFY(error.first().at(0).toString().contains("Qt error 6"));
    QVERIFY(QDir(pool.path()).entryList(QDir::Files | QDir::Hidden).isEmpty());
  }

  void timeout() {
    HttpServer server;
    Response response;
    response.hang = true;
    server.responses["/timeout.jpg"] = response;
    QTemporaryDir pool;
    FQTermHttp http(nullptr, nullptr, pool.path(), 0);
    QSignalSpy done(&http, &FQTermHttp::done), error(&http, &FQTermHttp::error);
    QSignalSpy preview(&http, &FQTermHttp::previewImage), message(&http, &FQTermHttp::message);
    QElapsedTimer elapsed;
    elapsed.start();
    http.getLink(server.url("/timeout.jpg"), true);
    QVERIFY(done.wait(35000));
    QVERIFY(elapsed.elapsed() >= 29000);
    QCOMPARE(error.count(), 1);
    QCOMPARE(preview.count(), 0);
    QCOMPARE(message.count(), 0);
  }
};

QTEST_MAIN(HttpTest)
#include "fqterm_http_test.moc"
