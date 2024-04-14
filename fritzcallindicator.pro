#  This file is part of FritzCallIndicator.
#  Copyright (C) 2012-present Thorsten Roth
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
#  along with FritzCallIndicator.  If not, see <http://www.gnu.org/licenses/>.

unix: !macx {
       TARGET  = fritzcallindicator
} else {
       TARGET  = FritzCallIndicator
}

win32:VERSION  = 0.1.0.0
else:VERSION   = 0.1.0

QMAKE_TARGET_PRODUCT     = "FritzCallIndicator"
QMAKE_TARGET_DESCRIPTION = "Simple FritzBox! call indicator"
QMAKE_TARGET_COPYRIGHT   = "(C) 2024-present Thorsten Roth"

DEFINES       += APP_NAME=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\" \
                 APP_VERSION=\"\\\"$$VERSION\\\"\" \
                 APP_DESC=\"\\\"$$QMAKE_TARGET_DESCRIPTION\\\"\" \
                 APP_COPY=\"\\\"$$QMAKE_TARGET_COPYRIGHT\\\"\"

MOC_DIR        = ./.moc
OBJECTS_DIR    = ./.objs
UI_DIR         = ./.ui
RCC_DIR        = ./.rcc

QT            += gui widgets network
CONFIG        += c++17
DEFINES       += QT_NO_FOREACH

CONFIG(debug, debug|release) {
  CONFIG      += warn_on
  DEFINES     += QT_DISABLE_DEPRECATED_BEFORE=0x060700
}

SOURCES       += main.cpp \
                 fritzcallindicator.cpp \
                 fritzbox.cpp \
                 settings.cpp

HEADERS       += fritzcallindicator.h \
                 fritzbox.h \
                 settings.h

FORMS         += settings.ui

RESOURCES      = data/data.qrc \
                 lang/translations.qrc

TRANSLATIONS  += lang/fritzcallindicator_de.ts \
                 lang/fritzcallindicator_en.ts

win32:RC_ICONS = icons/fritzcallindicator.ico

macx {
  ICON               = icons/icon.icns
  QMAKE_INFO_PLIST   = data/mac/Info.plist
}
