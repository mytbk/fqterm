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



#include <QEvent>
#include <QTextStream>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QMutex>
#include <QThread>
#include "fqterm.h"
#ifdef HAVE_PYTHON
#include <Python.h>
#endif
class SleeperThread : public QThread
{
public:
  static void msleep(unsigned long msecs)
  {
    QThread::msleep(msecs);
  }
};

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_PYTHON

#include "fqterm_window.h"
#include "fqterm_buffer.h"
#include "fqterm_text_line.h"
#include "fqterm_param.h"
#include "fqterm_session.h"
#include "fqterm_python.h"
#include "fqterm_path.h"
#include "common.h"
namespace FQTerm {

/* **************************************************************************
 *
 *				Pythons Embedding
 *
 * ***************************************************************************/

QString getException() {
  PyObject *pType = NULL,  *pValue = NULL,  *pTb = NULL,  *pName,  *pTraceback;

  PyErr_Fetch(&pType, &pValue, &pTb);

  pName = PyString_FromString("traceback");
  pTraceback = PyImport_Import(pName);
  Py_DECREF(pName);

  if (pTraceback == NULL) {
    return "General Error in Python Callback";
  }

  pName = PyString_FromString("format_exception");
  PyObject *pRes = PyObject_CallMethodObjArgs(pTraceback, pName, pType, pValue,
                                              pTb, NULL);
  Py_DECREF(pName);

  Py_DECREF(pTraceback);

  Py_XDECREF(pType);
  Py_XDECREF(pValue);
  Py_XDECREF(pTb);

  if (pRes == NULL) {
    return "General Error in Python Callback";
  }

  pName = PyString_FromString("string");
  PyObject *pString = PyImport_Import(pName);
  Py_DECREF(pName);

  if (pString == NULL) {
    return "General Error in Python Callback";
  }

  pName = PyString_FromString("join");
  PyObject *pErr = PyObject_CallMethodObjArgs(pString, pName, pRes, NULL);
  Py_DECREF(pName);

  Py_DECREF(pString);
  Py_DECREF(pRes);

  if (pErr == NULL) {
    return "General Error in Python Callback";
  }

  QString str(PyString_AsString(pErr));
  Py_DECREF(pErr);

  return str;
}

QString getErrOutputFile(FQTermWindow *lp) {
  // file name
  QString str2;
  str2.setNum(long(lp));
  str2 += ".err";
  // path
  return getPath(USER_CONFIG) + str2;
}

// copy current artcle for back compatible use only
// for new coder please use getArticle
static PyObject *fqterm_copyArticle(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  FQTermWindow *termWindow_ = (FQTermWindow*)lp;

  QStringList strList;
  QString strArticle;
  QReadWriteLock& bufferLock = termWindow_->getSession()->getBufferLock();
  QReadLocker locker(&bufferLock);
  while (1) {
    // check it there is duplicated string
    // it starts from the end in the range of one screen height
    // so this is a non-greedy match
    QString strTemp;
    termWindow_->getSession()->getBuffer()->getTextLineInTerm(0)->getAllPlainText(strTemp);
    strTemp = strTemp.trimmed();
    int i = 0;
    int start = 0;
    for (QStringList::Iterator it = strList.end();
         it != strList.begin() &&  i < termWindow_->getSession()->getBuffer()->getNumRows() - 1;  // not exceeeding the last screen
		 --it, i++) {
      if (*it != strTemp) {
        continue;
      }
      QStringList::Iterator it2 = it;
      bool dup = true;
      // match more to see if its duplicated
      for (int j = 0; j <= i; j++, it2++) {
        QString str1;
        termWindow_->getSession()->getBuffer()->getTextLineInTerm(j)->getAllPlainText(str1);
        if (*it2 != str1.trimmed()) {
          dup = false;
          break;
        }
      }
      if (dup) {
        // set the start point
        start = i + 1;
        break;
      }
    }
    // add new lines
    for (i = start; i < termWindow_->getSession()->getBuffer()->getNumRows() - 1; i++) {
      QString tmp;
      termWindow_->getSession()->getBuffer()->getTextLineInTerm(i)->getAllPlainText(tmp);
      strList += tmp.trimmed();
    }

    // the end of article
    QString testEnd;
    termWindow_->getSession()->getBuffer()->getTextLineInTerm(termWindow_->getSession()->getBuffer()->getNumRows() - 1)->getAllPlainText(testEnd);
    if (testEnd.indexOf("%") == -1) {
      break;
    }
    // continue
    termWindow_->writeString_ts(" ");

    // TODO: fixme
    if (!termWindow_->getSession()->getWaitCondition().wait(&bufferLock, 10000)) {
    //  // timeout
      break;
    }
  }
#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
  strArticle = strList.join("\r\n");
#else
  strArticle = strList.join("\n");
#endif

  PyObject *py_text = PyString_FromString(strArticle.toUtf8());

  Py_INCREF(py_text);
  return py_text;
}

static PyObject *fqterm_getArticle(PyObject *, PyObject *args) {
  long lp;
  int timeout;
  int succeed = 1;

  if (!PyArg_ParseTuple(args, "li", &lp, &timeout)) {
    return NULL;
  }
  FQTermWindow *termWindow_ = (FQTermWindow*)lp;

  QStringList strList;
  QString strArticle;
  QReadWriteLock& bufferLock = termWindow_->getSession()->getBufferLock();
  while (!bufferLock.tryLockForRead()) {}
  while (1) {
    // check it there is duplicated string
    // it starts from the end in the range of one screen height
    // so this is a non-greedy match
    QString strTemp;
    termWindow_->getSession()->getBuffer()->getTextLineInTerm(0)->getAllPlainText(strTemp);
    strTemp = strTemp.trimmed();

    int i = 0;
    int start = 0;
    for (QStringList::Iterator it = strList.end();
         it != strList.begin() && i < termWindow_->getSession()->getBuffer()->getNumRows() - 1;  // not exceeeding the last screen
         --it, i++) {
      if (it == strList.end() || *it != strTemp) {
        continue;
      }
      QStringList::Iterator it2 = it;
      bool dup = true;
      // match more to see if its duplicated
      for (int j = 0; j <= i && it2 != strList.end(); j++, it2++) {
        QString str1;
        termWindow_->getSession()->getBuffer()->getTextLineInTerm(j)->getAllPlainText(str1);
        if (*it2 != str1.trimmed()) {
          dup = false;
          break;
        }
      }
      if (dup) {
        // set the start point
        start = i + 1;
        break;
      }
    }
    // add new lines
    for (i = start; i < termWindow_->getSession()->getBuffer()->getNumRows() - 1; i++) {
      QString tmp;
      termWindow_->getSession()->getBuffer()->getTextLineInTerm(i)->getAllPlainText(tmp);
      strList += tmp.trimmed();
    }

    // the end of article
    QString testEnd;
    termWindow_->getSession()->getBuffer()->getTextLineInTerm(termWindow_->getSession()->getBuffer()->getNumRows() - 1)->getAllPlainText(testEnd);
    if (testEnd.indexOf("%") == -1) {
      break;
    }
    // continue
    termWindow_->writeString_ts(" ");

    // TODO: fixme
    if (!termWindow_->getSession()->getWaitCondition().wait(&bufferLock, timeout)) { // timeout
    	succeed = 0;
    	break;
    }
  }
  bufferLock.unlock();
  if (succeed)
    strArticle = strList.join(OS_NEW_LINE);

  PyObject *py_res = Py_BuildValue("si", (const char*)strArticle.toUtf8().data(),
                                   succeed);

  Py_INCREF(py_res);

  return py_res;

}

static PyObject *fqterm_formatError(PyObject *, PyObject *args) {
  long lp;

  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  QString strErr;
  QString filename = getErrOutputFile((FQTermWindow*)lp);

  QDir d;
  if (d.exists(filename)) {
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QTextStream is(&file);
    while (!is.atEnd()) {
      strErr += is.readLine(); // line of text excluding '\n'
      strErr += '\n';
    }
    file.close();
    d.remove(filename);
  }

  if (!strErr.isEmpty()) {
    ((FQTermWindow*)lp)->getPythonErrorMessage() = strErr;
    // TODO: fixme
    //qApp->postEvent((FQTermWindow*)lp, new QCustomEvent(PYE_ERROR));
  } else {
    // TODO: fixme
    //qApp->postEvent((FQTermWindow*)lp, new QCustomEvent(PYE_FINISH));
  }


  Py_INCREF(Py_None);
  return Py_None;
}

// caret x
static PyObject *fqterm_caretX(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }
  int x = ((FQTermWindow*)lp)->getSession()->getBuffer()->getCaretColumn();
  PyObject *py_x = Py_BuildValue("i", x);
  Py_INCREF(py_x);
  return py_x;
}

// caret y
static PyObject *fqterm_caretY(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }
  int y = ((FQTermWindow*)lp)->getSession()->getBuffer()->getCaretRow();
  PyObject *py_y = Py_BuildValue("i", y);
  Py_INCREF(py_y);
  return py_y;

}

// columns
static PyObject *fqterm_columns(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  int numColumns = ((FQTermWindow*)lp)->getSession()->getBuffer()->getNumColumns();
  PyObject *py_columns = Py_BuildValue("i", numColumns);

  Py_INCREF(py_columns);
  return py_columns;

}

// rows
static PyObject *fqterm_rows(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  int rows = ((FQTermWindow*)lp)->getSession()->getBuffer()->getNumRows();
  PyObject *py_rows = Py_BuildValue("i", rows);

  Py_INCREF(py_rows);
  return py_rows;
}

// sned string to server
static PyObject *fqterm_sendString(PyObject *, PyObject *args) {
  char *pstr;
  long lp;

  if (!PyArg_ParseTuple(args, "ls", &lp, &pstr)) {
    return NULL;
  }

  ((FQTermWindow*)lp)->writeRawString_ts(U82U(pstr));

  Py_INCREF(Py_None);
  return Py_None;
}

// input should be utf8.
// same as above except parsing string first "\n" "^p" etc
static PyObject *fqterm_sendParsedString(PyObject *, PyObject *args) {
  char *pstr;
  long lp;
  int len;

  if (!PyArg_ParseTuple(args, "ls", &lp, &pstr)) {
    return NULL;
  }
  len = strlen(pstr);

  ((FQTermWindow*)lp)->writeString_ts(U82U(pstr));

  Py_INCREF(Py_None);
  return Py_None;
}

// get text at line
static PyObject *fqterm_getText(PyObject *, PyObject *args) {
  long lp;
  int numRows;
  if (!PyArg_ParseTuple(args, "li", &lp, &numRows)) {
    return NULL;
  }
  QString str;
  if (numRows < ((FQTermWindow*)lp)->getSession()->getBuffer()->getNumRows())
    ((FQTermWindow*)lp)->getSession()->getBuffer()->getTextLineInTerm(numRows)->getAllPlainText(str);

  PyObject *py_text = PyString_FromString(U2U8(str));

  Py_INCREF(py_text);
  return py_text;
}

// get text with attributes
static PyObject *fqterm_getAttrText(PyObject *, PyObject *args) {
  long lp;
  int numRows;
  if (!PyArg_ParseTuple(args, "li", &lp, &numRows)) {
    return NULL;
  }
  
  QString str;
  if (numRows < ((FQTermWindow*)lp)->getSession()->getBuffer()->getNumRows())
    ((FQTermWindow*)lp)->getSession()->getBuffer()->getTextLineInTerm(numRows)->getAllAnsiText(str);

  PyObject *py_text = PyString_FromString(U2U8(str));

  Py_INCREF(py_text);
  return py_text;
}

// is host connected
static PyObject *fqterm_isConnected(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  bool connected = ((FQTermWindow*)lp)->isConnected();
  PyObject *py_connected = Py_BuildValue("i", connected ? 1 : 0);

  Py_INCREF(py_connected);
  return py_connected;
}

// disconnect from host
static PyObject *fqterm_disconnect(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  ((FQTermWindow*)lp)->disconnect();

  Py_INCREF(Py_None);
  return Py_None;
}

// reconnect to host
static PyObject *fqterm_reconnect(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  ((FQTermWindow*)lp)->getSession()->reconnect();

  Py_INCREF(Py_None);
  return Py_None;
}

// bbs encoding 0-GBK 1-BIG5
//FIXME: UTF8 and HKSCS
static PyObject *fqterm_getBBSCodec(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  PyObject *py_codec = PyString_FromString(((FQTermWindow*)lp)
                                           ->getSession()->param().serverEncodingID_ == 0 ? "GBK" : "Big5");
  Py_INCREF(py_codec);

  return py_codec;
}

// host address
static PyObject *fqterm_getAddress(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  PyObject *py_addr = PyString_FromString(((FQTermWindow*)lp)
                                          ->getSession()->param().hostAddress_.toLocal8Bit());
  Py_INCREF(py_addr);
  return py_addr;
}

// host port number
static PyObject *fqterm_getPort(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  PyObject *py_port = Py_BuildValue("i", ((FQTermWindow*)lp)->getSession()->param().port_);
  Py_INCREF(py_port);
  return py_port;
}

// connection protocol 0-telnet 1-SSH1 2-SSH2
static PyObject *fqterm_getProtocol(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  PyObject *py_port = Py_BuildValue("i", ((FQTermWindow*)lp)
                                    ->getSession()->param().protocolType_);
  Py_INCREF(py_port);
  return py_port;
}

// key to reply msg
static PyObject *fqterm_getReplyKey(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  PyObject *py_key = PyString_FromString(((FQTermWindow*)lp)
                                         ->getSession()->param().replyKeyCombination_.toLocal8Bit());
  Py_INCREF(py_key);
  return py_key;
}

// url under mouse
static PyObject *fqterm_getURL(PyObject *, PyObject *args) {
  long lp;
  if (!PyArg_ParseTuple(args, "l", &lp)) {
    return NULL;
  }

  PyObject *py_url = PyString_FromString(((FQTermWindow*)lp)->getSession()->getUrl().toUtf8().constData());
  Py_INCREF(py_url);
  return py_url;
}

// preview image link
static PyObject *fqterm_previewImage(PyObject *, PyObject *args) {
  long lp;
  char *url;
  if (!PyArg_ParseTuple(args, "ls", &lp, &url)) {
    return NULL;
  }

  ((FQTermWindow*)lp)->getHttpHelper(url, true);

  Py_INCREF(Py_None);
  return Py_None;

}

// convert string from UTF8 to specified encoding
static PyObject *fqterm_fromUTF8(PyObject *, PyObject *args) {
  char *str,  *enc;
  if (!PyArg_ParseTuple(args, "ss", &str, &enc)) {
    return NULL;
  }
  QTextCodec *encodec = QTextCodec::codecForName(enc);
  QTextCodec *utf8 = QTextCodec::codecForName("utf8");

  PyObject *py_str = PyString_FromString(encodec->fromUnicode(utf8->toUnicode
                                                              (str)));
  Py_INCREF(py_str);
  return py_str;
}

// convert string from specified encoding to UTF8
static PyObject *fqterm_toUTF8(PyObject *, PyObject *args) {
  char *str,  *enc;
  if (!PyArg_ParseTuple(args, "ss", &str, &enc)) {
    return NULL;
  }
  QTextCodec *encodec = QTextCodec::codecForName(enc);
  QTextCodec *utf8 = QTextCodec::codecForName("utf8");

  PyObject *py_str = PyString_FromString(utf8->fromUnicode(encodec->toUnicode
                                                           (str)));
  Py_INCREF(py_str);
  return py_str;
}

static PyObject *fqterm_wait(PyObject *, PyObject *args) {
  long t;
  if (!PyArg_ParseTuple(args, "l", &t)) {
    return NULL;
  }
  SleeperThread::msleep(t);

  Py_INCREF(Py_None);
  return Py_None;
}

PyMethodDef fqterm_methods[] =  {
  {
    "formatError", (PyCFunction)fqterm_formatError, METH_VARARGS,
	"get the traceback info"
  }

  ,

  {
    "getArticle", (PyCFunction)fqterm_getArticle, METH_VARARGS,
	"copy current article"
  }

  ,

  {
    "copyArticle", (PyCFunction)fqterm_copyArticle, METH_VARARGS,
	"copy current article (obsolete)"
  }

  ,

  {
    "getText", (PyCFunction)fqterm_getText, METH_VARARGS, "get text at line#"
  }

  ,

  {
    "getAttrText", (PyCFunction)fqterm_getAttrText, METH_VARARGS,
	"get attr text at line#"
  }

  ,

  {
    "sendString", (PyCFunction)fqterm_sendString, METH_VARARGS,
	"send string to server"
  }

  ,

  {
    "sendParsedString", (PyCFunction)fqterm_sendParsedString, METH_VARARGS,
	"send string with escape"
  }

  ,

  {
    "caretX", (PyCFunction)fqterm_caretX, METH_VARARGS, "caret x"
  }

  ,

  {
    "caretY", (PyCFunction)fqterm_caretY, METH_VARARGS, "caret y"
  }

  ,

  {
    "columns", (PyCFunction)fqterm_columns, METH_VARARGS, "screen width"
  }

  ,

  {
    "rows", (PyCFunction)fqterm_rows, METH_VARARGS, "screen height"
  }

  ,

  {
    "isConnected", (PyCFunction)fqterm_isConnected, METH_VARARGS,
	"connected to server or not"
  }

  ,

  {
    "disconnect", (PyCFunction)fqterm_disconnect, METH_VARARGS,
	"disconnect from server"
  }

  ,

  {
    "reconnect", (PyCFunction)fqterm_reconnect, METH_VARARGS, "reconnect"
  }

  ,

  {
    "getBBSCodec", (PyCFunction)fqterm_getBBSCodec, METH_VARARGS,
	"get the bbs encoding, GBK or Big5"
  }

  ,

  {
    "getAddress", (PyCFunction)fqterm_getAddress, METH_VARARGS,
	"get the bbs address"
  }

  ,

  {
    "getPort", (PyCFunction)fqterm_getPort, METH_VARARGS, "get the bbs port number"
  }

  ,

  {
    "getProtocol", (PyCFunction)fqterm_getProtocol, METH_VARARGS,
	"get the bbs protocol, 0/1/2 TELNET/SSH1/SSH2"
  }

  ,

  {
    "getReplyKey", (PyCFunction)fqterm_getReplyKey, METH_VARARGS,
	"get the key to reply messages"
  }

  ,

  {
    "getURL", (PyCFunction)fqterm_getURL, METH_VARARGS,
	"get the url string under mouse"
  }

  ,

  {
    "previewImage", (PyCFunction)fqterm_previewImage, METH_VARARGS,
	"preview the image link"
  }

  ,

  {
    "fromUTF8", (PyCFunction)fqterm_fromUTF8, METH_VARARGS,
	"decode from utf8 to string in specified codec"
  }

  ,

  {
    "toUTF8", (PyCFunction)fqterm_toUTF8, METH_VARARGS,
	"decode from string in specified codec to utf8"
  }

  ,

  {
    "wait", (PyCFunction)fqterm_wait, METH_VARARGS,
      "wait for x ms"
  }

  ,

  {
    NULL, (PyCFunction)NULL, 0, NULL
  }
};



FQTermPythonHelper::FQTermPythonHelper() 
	: mainThreadState_(NULL) {

    // initialize Python
    Py_Initialize();
	// initialize thread support
	PyEval_InitThreads();
	// save a pointer to the main PyThreadState object
	mainThreadState_ = PyThreadState_Get();
    // add path
    PyRun_SimpleString("import sys\n");
    QString pathCmd;
    pathCmd = "sys.path.insert(0,'";
    pathCmd += getPath(RESOURCE)+"script')";
    PyRun_SimpleString(fq_strdup(pathCmd.toUtf8().data()));
    
    Py_InitModule4("fqterm", fqterm_methods,
			NULL,(PyObject*)NULL,PYTHON_API_VERSION);
    // release the lock
    PyEval_ReleaseLock();
}

FQTermPythonHelper::~FQTermPythonHelper() {
    // shut down the interpreter
    PyInterpreterState * mainInterpreterState = mainThreadState_->interp;
    // create a thread state object for this thread
    PyThreadState * myThreadState = PyThreadState_New(mainInterpreterState);
    PyThreadState_Swap(myThreadState);
    PyEval_AcquireLock();
    Py_Finalize();
}
  
}  // namespace FQTerm

#endif //HAVE_PYTHON
