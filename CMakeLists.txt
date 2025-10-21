cmake_minimum_required(VERSION 4.0)

# Project metadata

project(Zemita VERSION 0.1.0 LANGUAGES CXX)

# Toolchain-wide basics
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)
# Build toggles for core only
option(ZEMITA_BUILD_CLI "Build the myzip command-line tool" ON)
option(ZEMITA_INSTALL   "Enable install rules"               ON)

# Core code lives in src/
add_subdirectory(src)
