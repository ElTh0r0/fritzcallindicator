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
    VERSION 0.4.0
    DESCRIPTION "FritzBox! call indicator"
    LANGUAGES C CXX
)
set(PROJECT_COPYRIGHT "(C) 2024-present Thorsten Roth")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MAIN_VERSION Qt6)
set(QT_MINIMUM_VERSION 6.2)

find_package(${QT_MAIN_VERSION} ${QT_MINIMUM_VERSION}
    REQUIRED Core Gui Widgets Network Sql LinguistTools
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX ON)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(WINDOWS ON)
#elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
#    set(MACOS ON)
endif()
    
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(WINDOWS)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/data/win.rc")
    qt_add_executable(fritzcallindicator ${app_icon_resource_windows})
    set_target_properties(fritzcallindicator PROPERTIES
        OUTPUT_NAME FritzCallIndicator
        WIN32_EXECUTABLE TRUE
    )
#elseif(MACOS)
#    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
#    set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/icons/icon.icns")
#    set_source_files_properties(${app_icon_macos} PROPERTIES
#           MACOSX_PACKAGE_LOCATION "Resources")
#
#    # TODO: To be checked how to bundle ressources (data)
#    qt_add_executable(fritzcallindicator MACOSX_BUNDLE ${app_icon_macos})
else()
    qt_add_executable(fritzcallindicator)
endif()

target_compile_features(fritzcallindicator PUBLIC cxx_std_17)
target_compile_definitions(fritzcallindicator
    PRIVATE
        APP_NAME="${PROJECT_NAME}"
        APP_VERSION="${PROJECT_VERSION}"
        APP_COPY="${PROJECT_COPYRIGHT}"
        APP_DESC="${PROJECT_DESCRIPTION}"
        QT_NO_FOREACH
        $<$<CONFIG:DEBUG>:QT_DISABLE_DEPRECATED_BEFORE=0x060800>
)

target_link_libraries(fritzcallindicator PRIVATE
    ${QT_MAIN_VERSION}::Core
    ${QT_MAIN_VERSION}::Gui
    ${QT_MAIN_VERSION}::Widgets
    ${QT_MAIN_VERSION}::Network
    ${QT_MAIN_VERSION}::Sql
)

set(HEADERS
    fritzbox.h
    fritzcallindicator.h
    numberresolver.h
    onlineresolvers.h
    settings.h
    tbaddressbook.h
)

set(SOURCES
    main.cpp
    fritzbox.cpp
    fritzcallindicator.cpp
    numberresolver.cpp
    onlineresolvers.cpp
    settings.cpp
    tbaddressbook.cpp
)

set(FORMS
    settings.ui
)

set(RESOURCES
    data/data.qrc
    lang/translations.qrc
)

set(TRANSLATIONS
    lang/fritzcallindicator_de.ts
    lang/fritzcallindicator_en.ts
)

set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_SOURCE_DIR}/lang")
qt_add_lrelease(
    LRELEASE_TARGET fritzcallindicator_lrelease
    TS_FILES ${TRANSLATIONS}
)

# Run lrelease automatically
add_dependencies(fritzcallindicator fritzcallindicator_lrelease)

target_sources(fritzcallindicator PRIVATE ${HEADERS} ${SOURCES} ${FORMS} ${RESOURCES})

if(LINUX)
    include(GNUInstallDirs)
    install(TARGETS fritzcallindicator DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES data/unix/com.github.elth0r0.fritzcallindicator.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES data/unix/com.github.elth0r0.fritzcallindicator.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    install(DIRECTORY data/area_codes DESTINATION ${CMAKE_INSTALL_DATADIR}/fritzcallindicator)
    install(DIRECTORY data/online_resolvers DESTINATION ${CMAKE_INSTALL_DATADIR}/fritzcallindicator)
    install(FILES data/country_codes.csv DESTINATION ${CMAKE_INSTALL_DATADIR}/fritzcallindicator)
    install(DIRECTORY icons/hicolor DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
#   add_subdirectory(man)
endif()
