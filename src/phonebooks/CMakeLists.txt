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
    carddav.h
    fritzphonebook.h
    thunderbird.h
)

set(SOURCES
    carddav.cpp
    fritzphonebook.cpp
    thunderbird.cpp
)

target_sources(${target_name} PRIVATE ${HEADERS} ${SOURCES})
