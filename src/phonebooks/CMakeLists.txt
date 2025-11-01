# SPDX-FileCopyrightText: 2025 Thorsten Roth
# SPDX-License-Identifier: GPL-3.0-or-later

string(TOLOWER ${PROJECT_NAME} target_name)

set(HEADERS
    fritzphonebook.h
)
if(FRITZ_USE_CARDDAV_ADDRESSBOOK)
    list(APPEND HEADERS carddav.h)
endif()
if(FRITZ_USE_THUNDERBIRD_ADDRESSBOOK)
    list(APPEND HEADERS thunderbird.h)
    target_link_libraries(fritzcallindicator PRIVATE
        ${QT_MAIN_VERSION}::Sql
    )
endif()

set(SOURCES
    fritzphonebook.cpp
)
if(FRITZ_USE_CARDDAV_ADDRESSBOOK)
    list(APPEND HEADERS carddav.cpp)
endif()
if(FRITZ_USE_THUNDERBIRD_ADDRESSBOOK)
    list(APPEND HEADERS thunderbird.cpp)
endif()

target_sources(${target_name} PRIVATE ${HEADERS} ${SOURCES})
