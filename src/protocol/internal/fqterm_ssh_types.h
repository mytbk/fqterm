// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SSH_TYPES_H
#define FQTERM_SSH_TYPES_H

#include <QGlobalStatic>
#if !defined(Q_OS_WIN32) && !defined(_OS_WIN32_)
#include <sys/types.h>
#else 
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

#ifdef QT3_SUPPORT
typedef Q_UINT32 u_int32_t;
typedef Q_UINT64 u_int64_t;
#else 
typedef quint32 u_int32_t;
typedef quint64 u_int64_t;
#endif // QT3_SUPPORT

#endif  // Q_OS_WIN32

#endif  // FQTERM_SSH_TYPES_H
