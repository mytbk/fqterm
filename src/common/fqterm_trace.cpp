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

#include <string>

#ifdef _MSC_VER
#include <hash_set>
#else
#include <set>
#endif

#include "fqterm_trace.h"

namespace FQTerm {

int *getDefaultMaxTraceLevel() {
  static int max_trace_level = 10;
  return &max_trace_level;
}

int getMaxTraceLevel() {
  return *getDefaultMaxTraceLevel();
}

void setMaxTraceLevel(int max_trace_level) {
  *getDefaultMaxTraceLevel() = max_trace_level;
  FQ_TRACE("info", 0) << "The upper bound of maximum trace level is "
                      << FQTERM_MAX_TRACE_LEVEL;
  FQ_TRACE("info", 0) << "The maximum trace level is set to "
                      << max_trace_level;
}

#ifdef _MSC_VER
typedef stdext::hash_set<std::string> CategorySet;
#else
typedef std::set<std::string> CategorySet;
#endif

CategorySet *getAllowedCategories() {
  static CategorySet *allowed_categories
      = new CategorySet;
  return allowed_categories;
}

int isAllowedCategory(const char *category, int trace_level) {
  if (trace_level <= 0) return true;
  
   const CategorySet &categories = *getAllowedCategories();
   if (categories.empty() 
	|| categories.find(category) != categories.end()) {
     return true;
   }
   return false;
}

void addAllowedCategory(const char *category) {
  CategorySet &categories = *getAllowedCategories();

  categories.insert(category);
  
  FQ_TRACE("info", 0) << "Allow category: " << category;
}

// Event name from event type id (updated to Qt4.3.0).
static const char *kEventName[] = {
  "QEvent::None", // 0
  "QEvent::Timer", // 1
  "QEvent::MouseButtonPress", // 2
  "QEvent::MouseButtonRelease", // 3
  "QEvent::MouseButtonDblClick", // 4
  "QEvent::MouseMove", // 5
  "QEvent::KeyPress", // 6
  "QEvent::KeyRelease", // 7
  "QEvent::FocusIn", // 8
  "QEvent::FocusOut", // 9
  "QEvent::Enter", // 10
  "QEvent::Leave", // 11
  "QEvent::Paint", // 12
  "QEvent::Move", // 13
  "QEvent::Resize", // 14
  "None", // 15
  "None", // 16
  "QEvent::Show", // 17
  "QEvent::Hide", // 18
  "QEvent::Close", // 19
  "None", // 20
  "QEvent::ParentChange", // 21
  "None", // 22
  "None", // 23
  "QEvent::WindowActivate", // 24
  "QEvent::WindowDeactivate", // 25
  "QEvent::ShowToParent", // 26
  "QEvent::HideToParent", // 27
  "None", // 28
  "None", // 29
  "None", // 30
  "QEvent::Wheel", // 31
  "None", // 32
  "QEvent::WindowTitleChange", // 33
  "QEvent::WindowIconChange", // 34
  "QEvent::ApplicationWindowIconChange", // 35
  "QEvent::ApplicationFontChange", // 36
  "QEvent::ApplicationLayoutDirectionChange", // 37
  "QEvent::ApplicationPaletteChange", // 38
  "QEvent::PaletteChange", // 39
  "QEvent::Clipboard", // 40
  "None", // 41
  "None", // 42
  "QEvent::MetaCall", // 43
  "None", // 44
  "None", // 45
  "None", // 46
  "None", // 47
  "None", // 48
  "None", // 49
  "QEvent::SockAct", // 50
  "QEvent::ShortcutOverride", // 51
  "QEvent::DeferredDelete", // 52
  "None", // 53
  "None", // 54
  "None", // 55
  "None", // 56
  "None", // 57
  "None", // 58
  "None", // 59
  "QEvent::DragEnter", // 60
  "QEvent::DragMove", // 61
  "QEvent::DragLeave", // 62
  "QEvent::Drop", // 63
  "None", // 64
  "None", // 65
  "None", // 66
  "None", // 67
  "QEvent::ChildAdded", // 68
  "QEvent::ChildPolished", // 69
  "None", // 70
  "QEvent::ChildRemoved", // 71
  "None", // 72
  "None", // 73
  "QEvent::PolishRequest", // 74
  "QEvent::Polish", // 75
  "QEvent::LayoutRequest", // 76
  "QEvent::UpdateRequest", // 77
  "QEvent::UpdateLater", // 78
  "None", // 79
  "None", // 80
  "None", // 81
  "QEvent::ContextMenu", // 82
  "QEvent::InputMethod", // 83
  "None", // 84
  "None", // 85
  "QEvent::AccessibilityPrepare", // 86
  "QEvent::TabletMove", // 87
  "QEvent::LocaleChange", // 88
  "QEvent::LanguageChange", // 89
  "QEvent::LayoutDirectionChange", // 90
  "None", // 91
  "QEvent::TabletPress", // 92
  "QEvent::TabletRelease", // 93
  "None", // 94
  "None", // 95
  "QEvent::IconDrag", // 96
  "QEvent::FontChange", // 97
  "QEvent::EnabledChange", // 98
  "QEvent::ActivationChange", // 99
  "QEvent::StyleChange", // 100
  "QEvent::IconTextChange", // 101
  "QEvent::ModifiedChange", // 102
  "QEvent::WindowBlocked", // 103
  "QEvent::WindowUnblocked", // 104
  "QEvent::WindowStateChange", // 105
  "None", // 106
  "None", // 107
  "None", // 108
  "QEvent::MouseTrackingChange", // 109
  "QEvent::ToolTip", // 110
  "QEvent::WhatsThis", // 111
  "QEvent::StatusTip", // 112
  "QEvent::ActionChanged", // 113
  "QEvent::ActionAdded", // 114
  "QEvent::ActionRemoved", // 115
  "QEvent::FileOpen", // 116
  "QEvent::Shortcut", // 117
  "QEvent::WhatsThisClicked", // 118
  "QEvent::AccessibilityHelp", // 119
  "QEvent::ToolBarChange", // 120
  "QEvent::ApplicationActivate", // 121
  "QEvent::ApplicationDeactivate", // 122
  "QEvent::QueryWhatsThis", // 123
  "QEvent::EnterWhatsThisMode", // 124
  "QEvent::LeaveWhatsThisMode", // 125
  "QEvent::ZOrderChange", // 126
  "QEvent::HoverEnter", // 127
  "QEvent::HoverLeave", // 128
  "QEvent::HoverMove", // 129
  "QEvent::AccessibilityDescription", // 130
  "QEvent::ParentAboutToChange", // 131
  "QEvent::WinEventAct", // 132
  "None", // 133
  "None", // 134
  "None", // 135
  "None", // 136
  "None", // 137
  "None", // 138
  "None", // 139
  "None", // 140
  "None", // 141
  "None", // 142
  "None", // 143
  "None", // 144
  "None", // 145
  "None", // 146
  "None", // 147
  "None", // 148
  "None", // 149
  "QEvent::EnterEditFocus", // 150
  "QEvent::LeaveEditFocus", // 151
  "None", // 152
  "QEvent::MenubarUpdated", // 153
  "None", // 154
  "QEvent::GraphicsSceneMouseMove", // 155
  "QEvent::GraphicsSceneMousePress", // 156
  "QEvent::GraphicsSceneMouseRelease", // 157
  "QEvent::GraphicsSceneMouseDoubleClick", // 158
  "QEvent::GraphicsSceneContextMenu", // 159
  "QEvent::GraphicsSceneHoverEnter", // 160
  "QEvent::GraphicsSceneHoverMove", // 161
  "QEvent::GraphicsSceneHoverLeave", // 162
  "QEvent::GraphicsSceneHelp", // 163
  "QEvent::GraphicsSceneDragEnter", // 164
  "QEvent::GraphicsSceneDragMove", // 165
  "QEvent::GraphicsSceneDragLeave", // 166
  "QEvent::GraphicsSceneDrop", // 167
  "QEvent::GraphicsSceneWheel", // 168
  "QEvent::KeyboardLayoutChange", // 169
  "QEvent::DynamicPropertyChange", // 170
  "QEvent::TabletEnterProximity", // 171
  "QEvent::TabletLeaveProximity", // 172
  // "QEvent::User", // 1000
  // "QEvent::MaxUser", // 65535
};

const char *getEventName(unsigned int type) {
  if (type > 1000) {
    return "User defined event";
  }

  if (type > 172) {
    return "None";
  }

  return kEventName[type];
}

}  // namespace FQTerm

#ifdef WIN32
/*
 * Number of micro-seconds between the beginning of the Windows epoch
 * (Jan. 1, 1601) and the Unix epoch (Jan. 1, 1970).
 *
 * This assumes all Win32 compilers have 64-bit support.
 */

#include <windows.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) || defined(__WATCOMC__)
  #define DELTA_EPOCH_IN_USEC  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_USEC  11644473600000000ULL
#endif

static unsigned long long filetime_to_unix_epoch (const FILETIME *ft)
{
    unsigned long long res = (unsigned long long) ft->dwHighDateTime << 32;

    res |= ft->dwLowDateTime;
    res /= 10;                   /* from 100 nano-sec periods to usec */
    res -= DELTA_EPOCH_IN_USEC;  /* from Win epoch to Unix epoch */
    return (res);
}

int gettimeofday (struct timeval *tv, int unused)
{
    FILETIME  ft;
    unsigned long long tim;

    GetSystemTimeAsFileTime (&ft);
    tim = filetime_to_unix_epoch (&ft);
    tv->tv_sec  = (long) (tim / 1000000L);
    tv->tv_usec = (long) (tim % 1000000L);
    return (0);
}
#else

#include <sys/time.h>
#include <time.h>

#endif

namespace FQTerm {

long long getTraceTime() {
  struct timeval t;
  gettimeofday(&t, 0);
  long long sec = t.tv_sec;
  long long usec = t.tv_usec;
  return sec * 1000 + usec/1000;
}

}  // namespace FQTerm
