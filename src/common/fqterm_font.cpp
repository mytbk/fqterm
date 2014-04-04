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

#include <list>
#include <set>
#include <utility>
#include <map>

#include <QFontDatabase>
#include <QFontInfo>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QLocale>

#include "fqterm_trace.h"
#include "fqterm_font.h"
#include "fqterm_path.h"
#include "fqterm_config.h"

using namespace std;

namespace FQTerm {

namespace Font {

enum Language{US_ENGLISH = 0,
              SIMPLIFIED_CHINESE,
              LANGUAGE_COUNT };

QString getLanguageName(Language lang) {
  switch (lang){
    case US_ENGLISH:
      return "en_US";
    case SIMPLIFIED_CHINESE:
      return "zh_CN";
    default:
      return "C";
  }
}

Language getLanguageByName(QString langName) {
  if (langName == "zh_CN") {
    return SIMPLIFIED_CHINESE;
  }
  if (langName == "en_US") {
    return US_ENGLISH;
  }
  return LANGUAGE_COUNT;
}

}  // namespace Language

typedef set<Font::Language> LanguageSet;
typedef map<QString, pair<QString, LanguageSet> >  FontLanguagesList;

typedef map<QString, map<Font::Language, QStringList> > FontSets;

// This function will be called only once.
static const FontSets &getPreferedFontSets() {
  static FontSets font_sets;
  const QString &res_path = getPath(RESOURCE);

  QString filename = res_path + "default_font.conf";

  if (!QFile::exists(filename)) {
    filename = res_path + "/dict/" + "default_font.conf";
  }


  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    FQ_TRACE("font", 0) << "Failed to open the default font configurations file:"
                        << filename;
    return font_sets;
  }

  QTextStream is;
  is.setDevice(&file);
  is.setCodec(QTextCodec::codecForName("UTF-8"));

#if defined(WIN32)
  QString expected_section = "Windows";
#elif defined(__APPLE__)
  QString expected_section = "Apple";
#else
  QString expected_section = "Linux";
#endif

  QString line;

  QString current_section;
  while (!is.atEnd()) {
    line = is.readLine().trimmed();
    if (line.isEmpty() || line[0] == '#') {
      continue;
    }

    if (line.left(1) == "[" && line.right(1) == "]") {
      current_section = line.mid(1, line.length() - 2);
      continue;
    }

    if (current_section == expected_section) {
      QString en_font = line.section('=', 0, 0).trimmed().toLower();
      QString fonts_for_lang = line.section('=', 1).trimmed();

      map<Font::Language, QStringList> &font_set = font_sets[en_font];

      QString lang = fonts_for_lang.section(":", 0, 0).trimmed();
      QString fonts = fonts_for_lang.section(":", 1).trimmed();

      QStringList font_list = fonts.split(",", QString::SkipEmptyParts);

      for (int i = 0; i < font_list.size(); ++i) {
        font_list[i] = font_list[i].trimmed();
      }

      font_set[Font::getLanguageByName(lang)] = font_list;
    }
  }

  return font_sets;
}

#if 0
//Not used
static int outputAllSystemFonts(const QStringList &fonts) {
  if (isAllowedCategory("font", 9)) {
    for (int i = 0; i < fonts.size(); ++i) {
      FQ_TRACE("font", 9) << "Found a font: "
                          << (QFontInfo(QFont(fonts.at(i))).fixedPitch() ?
                              "fixed-pitch     " : "variable-pitch  ")
                          << fonts.at(i);
    }
  }
  return 0;
}
#endif

static QStringList &getSystemFontFamilies() {
  static QStringList list = QFontDatabase().families();
  // static int tmp = outputAllSystemFonts(list);
  
  return list;
}

static FontSets &getUserConfigFontSets() {
  static FontSets font_sets = getPreferedFontSets();
  return font_sets;
}

static QString getEnglishFontFamily() {
  static const QStringList &families = getSystemFontFamilies();
  static const FontSets &font_sets = getUserConfigFontSets();
  static QString en_font_name;

  if (!en_font_name.isEmpty()) {
    return en_font_name;
  }

  FontSets::const_iterator it = font_sets.find("default");
  if (it != font_sets.end()) {
    map<Font::Language, QStringList>::const_iterator itt
        = it->second.find(Font::US_ENGLISH);
    if (itt != it->second.end()) {
      for (int i = 0; i < itt->second.size(); ++i) {
        const QString &font = itt->second[i];
        if (families.contains(font)) {
          en_font_name = font;
          break;
        }
      }
    }
  }

  return en_font_name;
}


static QString getFontFamilyForLang(Font::Language lang) {
  static const QStringList &families = getSystemFontFamilies();
  static const FontSets &font_sets = getUserConfigFontSets();

  const QString en_font_name = getEnglishFontFamily();

  if (lang == Font::US_ENGLISH) {
    return en_font_name;
  }

  FontSets::const_iterator it = font_sets.find(en_font_name.toLower());
  if (it != font_sets.end()) {
    map<Font::Language, QStringList>::const_iterator itt
        = it->second.find(lang);
    if (itt != it->second.end()) {
      for (int i = 0; i < itt->second.size(); ++i) {
        const QString &font = itt->second[i];
        if (families.contains(font)) {
          return font;
        } else {
          FQ_TRACE("font", 5) << "Font " << font << " for "
                              << en_font_name << " not found.";
        }
      }
    }
  } else {
    FQ_TRACE("font", 3) << "NO fontset for " << en_font_name << " found.";
  }

  return en_font_name;
}

QString getDefaultFontFamilyForLanguage(bool isEnglish) {
  Font::Language lang = isEnglish ? Font::US_ENGLISH : Font::SIMPLIFIED_CHINESE;
  
  static map<Font::Language, QString> langs_fonts_list;

  map<Font::Language, QString>::iterator it = langs_fonts_list.find(lang);
  if (it == langs_fonts_list.end()) {
    langs_fonts_list.insert(
        make_pair(lang, getFontFamilyForLang(lang)));
    it = langs_fonts_list.find(lang);

    FQ_TRACE("font", 3)  << "Defaut font for "
                         << Font::getLanguageName(lang)
                         << " is: "
                         << it->second
                         << " (" << it->second.size() << ")";
  }

  return it->second;
}

#if 0
//Not used.
static Font::Language getCurrentSystemLanguage() {
  if (QLocale::system().language() == QLocale::Chinese) {
    return Font::SIMPLIFIED_CHINESE;
  }

  return Font::US_ENGLISH;
}
#endif
}  // namespace FQTerm
