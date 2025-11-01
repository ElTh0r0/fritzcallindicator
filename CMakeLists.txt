# SPDX-FileCopyrightText: 2025 Thorsten Roth
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)
project(FritzCallIndicator
    VERSION 0.8.0
    DESCRIPTION "Fritz!Box call indicator"
    LANGUAGES C CXX
)
set(PROJECT_COPYRIGHT "(C) 2024-present Thorsten Roth")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MAIN_VERSION Qt6)
set(QT_MINIMUM_VERSION 6.2)

find_package(${QT_MAIN_VERSION} ${QT_MINIMUM_VERSION}
    REQUIRED Core Gui Widgets Network LinguistTools Xml
)

option(FRITZ_USE_ONLINE_RESOLVERS "Enable online resolvers" ON)
option(FRITZ_USE_THUNDERBIRD_ADDRESSBOOK "Enable Thunderbird addressbook" ON)
option(FRITZ_USE_CARDDAV_ADDRESSBOOK "Enable CardDAV addressbook" ON)
option(FRITZ_USE_NOTIFICATION_SOUND "Enable notification sound option" ON)
option(GENERATE_TS "Regenerate translation source files" OFF)

if (FRITZ_USE_ONLINE_RESOLVERS)
  add_compile_definitions(FRITZ_USE_ONLINE_RESOLVERS)
endif()
if (FRITZ_USE_THUNDERBIRD_ADDRESSBOOK)
  add_compile_definitions(FRITZ_USE_THUNDERBIRD_ADDRESSBOOK)
  find_package(${QT_MAIN_VERSION} ${QT_MINIMUM_VERSION}
      REQUIRED Sql
  )
endif()
if (FRITZ_USE_CARDDAV_ADDRESSBOOK)
  add_compile_definitions(FRITZ_USE_CARDDAV_ADDRESSBOOK)
endif()
if (FRITZ_USE_NOTIFICATION_SOUND)
  add_compile_definitions(FRITZ_USE_NOTIFICATION_SOUND)
  find_package(${QT_MAIN_VERSION} ${QT_MINIMUM_VERSION}
      REQUIRED Multimedia
  )
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(src)
