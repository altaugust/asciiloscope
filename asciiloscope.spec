Name:           asciiloscope
Version:        1.0.0
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

%files
%license LICENSE
%doc README.md
%{_bindir}/asciiloscope

%changelog
* Mon Feb 09 2026 Pedro Augusto <pedroaugustop@protonmail.com> - 1.0.0-1
- Initial Gold Master release