// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdlib.h>

#if defined(WIN32)
#include <windows.h>
#include <shellapi.h>
#else
#include <sys/stat.h>
#include <errno.h>
#include <locale.h>
#endif

#if defined(FQTERM_USE_STATIC_QT)
  // You can change the definition of FQTERM_USE_STATIC_QT in 
  // fqterm/CMakeLists.txt.{linux, macos, win32}.

  // static link Qt4 plugins.
  void loadNecessaryQtPlugins() {}
  #include <QtPlugin>
  Q_IMPORT_PLUGIN(qkrcodecs)
  Q_IMPORT_PLUGIN(qcncodecs)
  Q_IMPORT_PLUGIN(qjpcodecs)
  Q_IMPORT_PLUGIN(qtwcodecs)
#if QT_VERSION < QT_VERSION_CHECK(4,8,5)
  Q_IMPORT_PLUGIN(qjpeg)
  Q_IMPORT_PLUGIN(qgif)
  Q_IMPORT_PLUGIN(qmng)
#endif
#else
  // dynamic link Qt4 plugins.
  #include <QPluginLoader>
  void loadNecessaryQtPlugins() {
    static QPluginLoader qkrcodecsLoader( "qkrcodecs" );
    static QPluginLoader qcncodecsLoader( "qcncodecs" );
    static QPluginLoader qjpcodecsLoader( "qjpcodecs" );
    static QPluginLoader qtwcodecsLoader( "qtwcodecs" );
    static QPluginLoader qjpegLoader("qjpeg");
    static QPluginLoader qgifLoader("qgif");
    static QPluginLoader qmngLoader("qmng");
   
    qkrcodecsLoader.load();
    qcncodecsLoader.load();
    qjpcodecsLoader.load();
    qtwcodecsLoader.load();
    qjpegLoader.load();
    qgifLoader.load();
    qmngLoader.load();
  }
#endif

#include <QApplication>
#include <QFontDatabase>
#include <QTextCodec>

#include "fqterm.h"
#include "fqterm_frame.h"
#include "fqterm_path.h"
#include "fqterm_trace.h"
#include "fqterm_config.h"
#include "fqterm_param.h"
#include "fqterm_text_line.h"



int main(int argc, char **argv) {
  QApplication a(argc, argv);
  a.setApplicationName("FQTerm");
  a.setApplicationVersion(FQTERM_VERSION_STRING);
  // Set trace categories and level.
  FQTerm::setMaxTraceLevel(1);
  for (int i = 1; i < argc; ++i) {
    QString str(argv[i]);
    bool ok;
    int max_level = str.toInt(&ok, 0); 

    if (ok) {
      FQTerm::setMaxTraceLevel(max_level);
    } else {
      FQTerm::addAllowedCategory(argv[i]);
    }
  }

  using namespace FQTerm;

  loadNecessaryQtPlugins();

  // char buf[] = "\xc8\xb8";
  // QString ucs2 = QTextCodec::codecForName("Big5")->toUnicode(buf);
  // QVector<uint> ucs4 = ucs2.toUcs4(); 
  // QByteArray utf8 = ucs2.toUtf8();
  // const char *utf8c = utf8.constData();

  // FQ_TRACE("text", 0) << "\n" << dumpHexString << std::string(buf, sizeof(buf) - 1)
  //                     << dumpNormalString << " \n-->"
  //                     << dumpNormalString << "\nucs4 " << ucs4.size() << " " << ucs4[0]
  //                     << dumpNormalString << "\nucs2 " << ucs2.size() << " " << ucs2.at(0).unicode()
  //                     << dumpNormalString << "\nutf8   " << dumpHexString << utf8c;
  //return 0;

  if (!iniSettings()) {
    return -1;
  }

  FQTermFrame *mw = new FQTermFrame();
  mw->setWindowTitle("FQTerm " + QString(FQTERM_VERSION_STRING));
  mw->setWindowIcon(QPixmap(getPath(RESOURCE) + "pic/fqterm.png"));
  mw->show();
  return a.exec();
}
