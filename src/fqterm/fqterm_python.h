// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_PYTHON_H
#define FQTERM_PYTHON_H

#ifdef HAVE_PYTHON
#include "fqterm.h"

namespace FQTerm {

class FQTermWindow;
extern QString getException();
extern QString getErrOutputFile(FQTermWindow*);
extern PyMethodDef fqterm_methods[];

class FQTermPythonHelper
{
public:
  FQTermPythonHelper();
  ~FQTermPythonHelper();
  PyThreadState* getPyThreadState() {
    return mainThreadState_;
  }
private:
  PyThreadState * mainThreadState_;
};

}  // namespace FQTerm

#endif  // HAVE_PYTHON

#endif  // FQTERM_PYTHON_H

