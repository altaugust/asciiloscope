Name:           asciiloscope
Version:        1.0.1
Release:        1%{?dist}
Summary:        Terminal-based audio oscilloscope visualizer

License:        MIT
URL:            https://github.com/altaugust/asciiloscope
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  ncurses-devel
BuildRequires:  pulseaudio-libs-devel

Requires:       ncurses
Requires:       pulseaudio-libs
Requires:       pulseaudio-utils

%description
Asciiloscope is a low-latency audio visualizer for the terminal.
It simulates the aesthetic of old CRT oscilloscopes, supporting
Waveform and XY (Lissajous) visualization modes.

%prep
%autosetup

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

* Mon Feb 09 2026 Pedro Augusto <pedroaugustop@protonmail.com> - 1.0.0-1
- Initial Gold Master release