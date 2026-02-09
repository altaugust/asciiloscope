cmake_minimum_required(VERSION 3.10)

# Project Metadata
project(asciiloscope
    VERSION 1.0.0
    DESCRIPTION "Terminal-based audio oscilloscope visualizer"
    LANGUAGES CXX
)

# C++ Standard Configuration (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Enforce standard C++, disables compiler-specific extensions

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

# 1. PulseAudio (via PkgConfig)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PULSE REQUIRED libpulse libpulse-simple)

# 2. Ncurses (via CMake Standard Module)
find_package(Curses REQUIRED)

# ------------------------------------------------------------------------------
# Build Targets
# ------------------------------------------------------------------------------

add_executable(asciiloscope main.cpp)

# Compiler Options (Enable warnings for code quality)
if(MSVC)
    target_compile_options(asciiloscope PRIVATE /W4)
else()
    target_compile_options(asciiloscope PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Include Directories (Target-specific)
target_include_directories(asciiloscope PRIVATE
    ${PULSE_INCLUDE_DIRS}
    ${CURSES_INCLUDE_DIRS}
)

# Link Libraries
target_link_libraries(asciiloscope PRIVATE
    ${PULSE_LIBRARIES}
    ${CURSES_LIBRARIES}
)

# ------------------------------------------------------------------------------
# Installation Rules (Critical for RPM/DEB Packaging)
# ------------------------------------------------------------------------------

include(GNUInstallDirs)

# Install binary to system binary directory (usually /usr/bin)
install(TARGETS asciiloscope
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)