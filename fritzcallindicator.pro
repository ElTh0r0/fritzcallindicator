#  This file is part of FritzCallIndicator.
#  Copyright (C) 2024-present Thorsten Roth
#
#  FritzCallIndicator is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  FritzCallIndicator is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with FritzCallIndicator.  If not, see <https://www.gnu.org/licenses/>.

unix: !macx {
       TARGET  = fritzcallindicator
} else {
       TARGET  = FritzCallIndicator
}

win32:VERSION  = 0.3.3.0
else:VERSION   = 0.3.3

QMAKE_TARGET_PRODUCT     = "FritzCallIndicator"
QMAKE_TARGET_DESCRIPTION = "FritzBox! call indicator"
QMAKE_TARGET_COPYRIGHT   = "(C) 2024-present Thorsten Roth"

DEFINES       += APP_NAME=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\" \
                 APP_VERSION=\"\\\"$$VERSION\\\"\" \
                 APP_DESC=\"\\\"$$QMAKE_TARGET_DESCRIPTION\\\"\" \
                 APP_COPY=\"\\\"$$QMAKE_TARGET_COPYRIGHT\\\"\"

MOC_DIR        = ./.moc
OBJECTS_DIR    = ./.objs
UI_DIR         = ./.ui
RCC_DIR        = ./.rcc

QT            += gui widgets network sql
CONFIG        += c++17
DEFINES       += QT_NO_FOREACH

CONFIG(debug, debug|release) {
  CONFIG      += warn_on
  DEFINES     += QT_DISABLE_DEPRECATED_BEFORE=0x060800
}

SOURCES       += main.cpp \
                 fritzcallindicator.cpp \
                 fritzbox.cpp \
                 numberresolver.cpp \
                 onlineresolvers.cpp \
                 settings.cpp \
                 tbaddressbook.cpp

HEADERS       += fritzcallindicator.h \
                 fritzbox.h \
                 numberresolver.h \
                 onlineresolvers.h \
                 settings.h \
                 tbaddressbook.h

FORMS         += settings.ui

RESOURCES      = data/data.qrc \
                 lang/translations.qrc

TRANSLATIONS  += lang/fritzcallindicator_de.ts \
                 lang/fritzcallindicator_en.ts

win32:RC_FILE  = data/win.rc

macx {
  ICON               = icons/icon.icns
  QMAKE_INFO_PLIST   = data/mac/Info.plist

  CODES_DATA.path    = Contents/Resources
  CODES_DATA.files  += data/area_codes
  CODES_DATA.files  += data/online_resolvers
  CODES_DATA.files  += data/country_codes.csv
  QMAKE_BUNDLE_DATA += CODES_DATA
}

unix: !macx {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    isEmpty(BINDIR) {
        BINDIR = bin
    }

    target.path    = $$PREFIX/$$BINDIR/

    data.path      = $$PREFIX/share/fritzcallindicator
    data.files    += data/area_codes
    data.files    += data/online_resolvers
    data.files    += data/country_codes.csv

    desktop.path   = $$PREFIX/share/applications
    desktop.files += data/unix/com.github.elth0r0.fritzcallindicator.desktop

    icons.path     = $$PREFIX/share/icons
    icons.files   += icons/hicolor

    meta.path      = $$PREFIX/share/metainfo
    meta.files    += data/unix/com.github.elth0r0.fritzcallindicator.metainfo.xml

    INSTALLS      += target \
                     data \
                     desktop \
                     icons \
                     meta
}
