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

#ifndef FQTERM_TRACE_H
#define FQTERM_TRACE_H

#include <cassert>
#ifndef WIN32
#include <unistd.h>
#endif // WIN32

#include <QtDebug>
#include <QFileInfo>

namespace FQTerm {

// define NO_FQTERM_TRACE to avoid all trace output.
// #define NO_FQTERM_TRACE

// define NO_FQTERM_TRACE_FILE_LINE_NUM to avoid file line number in trace
// output.

// define NO_FQTERM_TRACE_FUNCTION_NAME to avoid function name in trace output.
// define integer FQTERM_MAX_TRACE_LEVEL to determine the maximize outputed
// trace level.
//#define NO_FQTERM_TRACE
#if defined(NDEBUG)
#define FQTERM_MAX_TRACE_LEVEL 1
#endif

#ifndef FQTERM_MAX_TRACE_LEVEL
#define FQTERM_MAX_TRACE_LEVEL 20
#endif  // FQTERM_MAX_TRACE_LEVEL

int getMaxTraceLevel();

void setMaxTraceLevel(int max_trace_level);

long long getTraceTime();

int isAllowedCategory(const char *category, int trace_level);

void addAllowedCategory(const char *category);

static inline void soft_break() {
#ifdef WIN32
#ifdef _MSC_VER
    __asm int 03h;
#else
    asm("int $0x03");
#endif
#else
  pause();
#endif
}

template<int trace_level>
class FQTermTrace {
 public:
  FQTermTrace(const char *category, const char *file_name, int line_num, const char *func_name)
      : category_(category),
        is_dummy_(!isAllowedCategory(category, trace_level)),
        trace_output_((trace_level <= FQTERM_MAX_TRACE_LEVEL
                       && trace_level <= getMaxTraceLevel())
                      ? new QDebug(qDebug().nospace())
                      : NULL),
        dump_string_(false) {

    if (is_dummy_) return;

#ifdef NO_FQTERM_TRACE_FILE_LINE_NUM
#ifdef NO_FQTERM_TRACE_FUNCTION_NAME
    return;
#endif  // NO_FQTERM_TRACE_FUNCTION_NAME
#endif  // NO_FQTERM_TRACE_FILE_LINE_NUM

    (*this) << "[" << category_ << " " << trace_level << " ";

#ifndef NO_FQTERM_TRACE_FILE_LINE_NUM
    (*this) << QFileInfo(file_name).fileName().toLatin1().constData()
            << ": " << line_num;
#endif  // NO_FQTERM_TRACE_FILE_LINE_NUM

#ifndef NO_FQTERM_TRACE_FUNCTION_NAME
#ifndef NO_FQTERM_TRACE_FILE_LINE_NUM
    (*this) << ", ";
#endif  // NO_FQTERM_TRACE_FILE_LINE_NUM
    (*this) << func_name;
#endif  // NO_FQTERM_TRACE_FUNCTION_NAME

    (*this) << "] ";
  }

  ~FQTermTrace() {
    if (is_dummy_) return;

    if (trace_level <= FQTERM_MAX_TRACE_LEVEL && trace_level <= getMaxTraceLevel()) {
      delete trace_output_;

      if (trace_level < 0) {
        soft_break();
        assert(false);
        qFatal("Fatal error occured!\n");
      }
    }
  }

  template<class T>
  FQTermTrace &operator << (const T &t) {
    if (is_dummy_) return *this;
    
    if (trace_level <= FQTERM_MAX_TRACE_LEVEL && trace_level <= getMaxTraceLevel()) {
      *trace_output_ << t;
    }
    return *this;
  }

  FQTermTrace &operator << (const std::string &t) {
    return output_raw_string(t);
  }

  FQTermTrace &operator << (const char *t) {
    return output_raw_string(t);
  }

  FQTermTrace &operator << (void (*setter)(FQTermTrace<trace_level> &)) {
    (*setter)(*this);
    return *this;
  }

 private:
  FQTermTrace &output_raw_string(const std::string &t) {
    if (is_dummy_) return *this;
     
    if (trace_level <= FQTERM_MAX_TRACE_LEVEL && trace_level <= getMaxTraceLevel()) {
      if (!dump_string_) {
        for (std::string::size_type i = 0; i < t.size(); ++i) {
          *trace_output_ << t[i];
        }
      } else {
        for (std::string::size_type i = 0; i < t.size(); ++i) {
          unsigned char a = t[i];
          unsigned char b = a / 0x10;
          unsigned char c = a % 0x10;
          char h = (char) (b < 10 ? '0' + b : 'A' + b - 10);
          char l = (char) (c < 10 ? '0' + c : 'A' + c - 10);

          *trace_output_ << h << l << ' ';
        }
      }
    }
    return *this;
  }

 private:
  const char *category_;
  bool is_dummy_;
  QDebug *trace_output_;

 public:
  bool dump_string_; // whether we print string in hex mode.

};

template<int trace_level>
class FQTermDummyTrace {
 public:
  template<class T>
  FQTermDummyTrace &operator << (const T &) {
    return *this;
  }

  FQTermDummyTrace &operator << (void (*setter)(FQTermTrace<trace_level> &)) {
    return *this;
  }
};

template<int trace_level>
void dumpHexString(FQTermTrace<trace_level> &tracer) {
  tracer.dump_string_ = true;
}

template<int trace_level>
void dumpNormalString(FQTermTrace<trace_level> &tracer) {
  tracer.dump_string_ = false;
}

template<int trace_level>
class FQTermScopeTrace {
 public:
  FQTermScopeTrace(const char *category, const char *file_name, int line_num,
                   const char *func_name, const char *scope_name)
      : category_(category),
        is_dummy_(!isAllowedCategory(category, trace_level)),        
        file_name_(file_name),
        line_num_(line_num),
        func_name_(func_name),
        scope_name_(scope_name) {
    if (is_dummy_) return;
    FQTermTrace<trace_level>(category_, file_name_, line_num_, func_name_)
        << "Entering " << scope_name_;
  }

  ~FQTermScopeTrace() {
    if (is_dummy_) return;
    FQTermTrace<trace_level>(category_, file_name_, line_num_, func_name_)
        << "Leaving " << scope_name_;
  }

 private:
  const char *category_;
  bool is_dummy_;

  const char *file_name_;
  const int line_num_;
  const char *func_name_;
  const char *scope_name_;
};

template<int trace_level>
class FQTermTimerTrace {
 public:
  FQTermTimerTrace(const char *category, const char *file_name, int line_num,
                   const char *func_name, const char *msg)
      : category_(category),
        is_dummy_(!isAllowedCategory(category, trace_level)),
        file_name_(file_name),
        line_num_(line_num),
        func_name_(func_name),
        msg_(msg),
        start_time(is_dummy_? 0: getTraceTime()) {
  }

  ~FQTermTimerTrace() {
    if (is_dummy_) return;

    long long total_time = getTraceTime() - start_time;

    long long num_sec = total_time / 1000;
    long long num_ms = total_time % 1000;

    FQTermTrace<trace_level>(category_, file_name_, line_num_, func_name_)
        << msg_ << num_sec << "s" << num_ms << "ms";
  }

 private:
  const char *category_;
  bool is_dummy_;

  const char *file_name_;
  const int line_num_;
  const char *func_name_;
  const char *msg_;

  long long start_time; // in millisecond.
};

// Some macros work in both debug & release version.
#define FQ_FATAL(file, line, func)                                      \
  do {FQTerm::soft_break(); qFatal("%s", (QString(file) + ": %1 " + QString(func)).arg(line).toUtf8().constData());} while(false)

#define FQ_VERIFY(expr) do {                        \
    if(!(expr)) {                                   \
      FQ_FATAL(__FILE__, __LINE__, __FUNCTION__);   \
    }                                               \
  } while(false)



#ifdef NO_FQTERM_TRACE

#define FQ_TRACE(category, trace_level) FQTermDummyTrace<trace_level>()
#define FQ_SCOPE_TRACE(category, trace_level, scope_name) do {} while (false)
#define FQ_FUNC_TRACE(category, trace_level) do {} while(false)

#define FQ_ASSERT(expr) do {} while(false)

#define FQ_SCOPE_TIMER(category, trace_level, msg) do {} while(false)
#define FQ_FUNC_TIMER(category, trace_level) do {} while(false)


#else // NO_FQTERM_TRACE

#define FQ_TRACE(category, trace_level)                                 \
  FQTermTrace<trace_level>(category, __FILE__, __LINE__, __FUNCTION__)

#define FQ_SCOPE_TRACE_PRIVATE(file, line, func, category, trace_level, scope_name) \
  FQTermScopeTrace<trace_level> file##line##func(category, file, line, func, scope_name);

#define FQ_SCOPE_TRACE(category, trace_level, scope_name)               \
  FQ_SCOPE_TRACE_PRIVATE(__FILE__, __LINE__, __FUNCTION__, category, trace_level, scope_name)

#define FQ_FUNC_TRACE(category, trace_level)                            \
  FQ_SCOPE_TRACE_PRIVATE(__FILE__, __LINE__, __FUNCTION__, category, trace_level, "function")


#define FQ_ASSERT(expr)                                                 \
  do {if(!(expr)) {FQ_TRACE("assert", -1) << "assertion failed." << #expr;}} while(false)


#define FQ_SCOPE_TIMER_PRIVATE(file, line, func, category, trace_level, msg) \
  FQTermTimerTrace<trace_level> file##line##func(category, file, line, func, msg);

#define FQ_SCOPE_TIMER(category, trace_level, msg)                      \
  FQ_SCOPE_TIMER_PRIVATE(__FILE__, __LINE__, __FUNCTION__, category, trace_level, msg)

#define FQ_FUNC_TIMER(category, trace_level)                            \
  FQ_SCOPE_TIMER_PRIVATE(__FILE__, __LINE__, __FUNCTION__, category, trace_level, "The function runs for ")


#endif  // NO_FQTERM_TRACE


const char *getEventName(unsigned int type);

}  // namespace FQTerm

#endif  // FQTERM_TRACE_H
