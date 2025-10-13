cmake_minimum_required(VERSION 3.20)

# Project metadata
project(Zemita VERSION 0.1.0 LANGUAGES CXX)

# Toolchain-wide basics
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build toggles for core only
option(ZEMITA_BUILD_CLI "Build the myzip command-line tool" ON)
option(ZEMITA_INSTALL   "Enable install rules"               ON)

# Core code lives in src/
add_subdirectory(src)