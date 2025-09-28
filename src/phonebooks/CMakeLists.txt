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
