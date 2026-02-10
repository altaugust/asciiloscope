Name:           asciiloscope
Version:        1.0.1
Release:        1%{?dist}
Summary:        A real-time, low-latency audio visualizer for the Linux terminal

License:        MIT
URL:            https://github.com/altaugust/asciiloscope
Source0:        %{url}/archive/main.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  ncurses-devel
BuildRequires:  pulseaudio-libs-devel

%description
Asciiloscope is a terminal-based audio visualizer that simulates the aesthetic 
of vintage CRT oscilloscopes using ASCII characters.

%prep
%setup -q -n asciiloscope-main

%build
%cmake
%cmake_build

%install
%cmake_install
install -Dp -m 644 debian/asciiloscope.1 %{buildroot}%{_mandir}/man1/asciiloscope.1

%files
%license LICENSE
%doc README.md
%{_bindir}/asciiloscope
%{_mandir}/man1/asciiloscope.1*

%changelog
* Tue Feb 10 2026 Pedro Augusto <pedroaugustop@protonmail.com> - 1.0.1-1
- Added official manpage documentation
- Updated build dependencies for Fedora

* Mon Feb 09 2026 Pedro Augusto <pedroaugustop@protonmail.com> - 1.0.0-1
- Initial Gold Master release