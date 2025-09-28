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

cmake_minimum_required(VERSION 3.16)
project(FritzCallIndicator
    VERSION 0.7.0
    DESCRIPTION "Fritz!Box call indicator"
    LANGUAGES C CXX
)
set(PROJECT_COPYRIGHT "(C) 2024-present Thorsten Roth")

option(FRITZ_USE_ONLINE_RESOLVERS "Enable online resolvers" ON)
option(FRITZ_USE_THUNDERBIRD_ADDRESSBOOK "Enable Thunderbird addressbook" ON)
option(GENERATE_TS "Regenerate translation source files" OFF)

if (FRITZ_USE_ONLINE_RESOLVERS)
  add_compile_definitions(FRITZ_USE_ONLINE_RESOLVERS)
endif()
if (FRITZ_USE_THUNDERBIRD_ADDRESSBOOK)
  add_compile_definitions(FRITZ_USE_THUNDERBIRD_ADDRESSBOOK)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MAIN_VERSION Qt6)
set(QT_MINIMUM_VERSION 6.2)

find_package(${QT_MAIN_VERSION} ${QT_MINIMUM_VERSION}
    REQUIRED Core Gui Widgets Network Sql LinguistTools Xml
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(src)
