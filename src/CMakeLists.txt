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

if(WIN32)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/../data/win.rc")
    qt_add_executable(fritzcallindicator ${app_icon_resource_windows})
    set_target_properties(fritzcallindicator PROPERTIES
        OUTPUT_NAME FritzCallIndicator
        WIN32_EXECUTABLE TRUE
    )
#elseif(APPLE)
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
        $<$<CONFIG:DEBUG>:QT_DISABLE_DEPRECATED_BEFORE=0x060900>
)

target_link_libraries(fritzcallindicator PRIVATE
    ${QT_MAIN_VERSION}::Core
    ${QT_MAIN_VERSION}::Gui
    ${QT_MAIN_VERSION}::Network
    ${QT_MAIN_VERSION}::Widgets
    ${QT_MAIN_VERSION}::Xml
)
if(FRITZ_USE_NOTIFICATION_SOUND)
    target_link_libraries(fritzcallindicator PRIVATE
        ${QT_MAIN_VERSION}::Multimedia
    )
endif()

add_subdirectory(phonebooks)

set(SOURCES
    main.cpp
    callmonitor.cpp
    fritzcallindicator.cpp
    fritzsoap.cpp
    numberresolver.cpp
    settings.cpp
    settingsdialog.cpp
)
if(FRITZ_USE_ONLINE_RESOLVERS)
    list(APPEND SOURCES onlineresolvers.cpp)
endif()

set(HEADERS
    callmonitor.h
    fritzcallindicator.h
    fritzsoap.h
    numberresolver.h
    settings.h
    settingsdialog.h
)
if(FRITZ_USE_ONLINE_RESOLVERS)
    list(APPEND HEADERS onlineresolvers.h)
endif()

set(FORMS
    settingsdialog.ui
)

set(RESOURCES
    ../data/data.qrc
)

set(TRANSLATIONS
    ../lang/fritzcallindicator_de.ts
    ../lang/fritzcallindicator_en.ts
)

if (GENERATE_TS)
    qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TRANSLATIONS} OPTIONS -no-obsolete)
else ()
    qt6_add_translation(QM_FILES ${TRANSLATIONS})
endif ()
add_custom_target(translations_${PROJECT_NAME} ALL DEPENDS ${QM_FILES})
foreach(qm_file ${QM_FILES})
    get_filename_component(qm_filename "${qm_file}" NAME)
    set_source_files_properties(
        "${qm_file}"
        PROPERTIES QT_RESOURCE_ALIAS "${qm_filename}"
    )
endforeach()
qt_add_resources(fritzcallindicator translations_fritz
    PREFIX "/"
    FILES ${QM_FILES}
)

target_sources(fritzcallindicator PRIVATE ${HEADERS} ${SOURCES} ${FORMS} ${RESOURCES})

if(LINUX)
    include(GNUInstallDirs)
    install(TARGETS fritzcallindicator DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${CMAKE_SOURCE_DIR}/data/unix/com.github.elth0r0.fritzcallindicator.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES ${CMAKE_SOURCE_DIR}/data/unix/com.github.elth0r0.fritzcallindicator.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/area_codes DESTINATION ${CMAKE_INSTALL_DATADIR}/fritzcallindicator)
    if(FRITZ_USE_ONLINE_RESOLVERS)
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/online_resolvers DESTINATION ${CMAKE_INSTALL_DATADIR}/fritzcallindicator)
    endif()
    install(FILES ${CMAKE_SOURCE_DIR}/data/country_codes.csv DESTINATION ${CMAKE_INSTALL_DATADIR}/fritzcallindicator)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/icons/hicolor DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
#   add_subdirectory(man)
endif()
