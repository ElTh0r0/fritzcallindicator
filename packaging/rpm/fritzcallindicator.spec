#
# spec file for FritzCallIndicator
#
# Copyright (C) 2024-present Thorsten Roth
#
# FritzCallIndicator is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# FritzCallIndicator is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with FritzCallIndicator.  If not, see <http://www.gnu.org/licenses/>.

Name:           fritzcallindicator
Summary:        Fritz!Box call indicator
Version:        0.7.0
Release:        1
License:        GPL-3.0+
URL:            https://github.com/ElTh0r0/fritzcallindicator
Source:         %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-build
Group:          Applications/Internet

#--------------------------------------------------------------------
# Fedora
#--------------------------------------------------------------------
%if 0%{?fedora}
BuildRequires:  desktop-file-utils
BuildRequires:  gettext
BuildRequires:  libappstream-glib
BuildRequires:  ninja-build
# Fedora <= 41
BuildRequires: libvpl
# Fedora >= 42
BuildRequires: ocl-icd
%endif
#--------------------------------------------------------------------
# All
#--------------------------------------------------------------------
BuildRequires:  gcc-c++
BuildRequires:  hicolor-icon-theme
BuildRequires:  cmake
BuildRequires:  cmake(Qt6Core)
BuildRequires:  cmake(Qt6Gui)
BuildRequires:  cmake(Qt6LinguistTools)
BuildRequires:  cmake(Qt6Widgets)
BuildRequires:  cmake(Qt6Sql)
BuildRequires:  cmake(Qt6Multimedia)
BuildRequires:  cmake(Qt6Xml)
#--------------------------------------------------------------------

%description
Show taskbar notifications for incoming calls from the Fritz!Box.

%prep
%autosetup -p1

#--------------------------------------------------------------------
# Fedora
#--------------------------------------------------------------------
%if 0%{?fedora}
%build
%cmake_qt6
%cmake_build

%install
%cmake_install

%check
desktop-file-validate %{buildroot}/%{_datadir}/applications/com.github.elth0r0.fritzcallindicator.desktop || :
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/com.github.elth0r0.fritzcallindicator.metainfo.xml || :
%endif
#--------------------------------------------------------------------
# SUSE
#--------------------------------------------------------------------
%if 0%{?suse_version}
%build
%cmake_qt6
%{qt6_build}

%install
%{qt6_install}
%endif
#--------------------------------------------------------------------

%files
%defattr(-,root,root,-)
%if 0%{?suse_version}
%dir %{_datadir}/metainfo
%{_datadir}/icons/hicolor/
%endif
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/com.github.elth0r0.fritzcallindicator.desktop
%{_datadir}/icons/hicolor/*/apps/com.github.elth0r0.fritzcallindicator.*g
%{_datadir}/metainfo/com.github.elth0r0.fritzcallindicator.metainfo.xml
%doc COPYING

%changelog
