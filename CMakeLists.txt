cmake_minimum_required(VERSION 4.0)

# Project metadata

project(Zemita VERSION 0.1.0 LANGUAGES CXX)

# Toolchain-wide basics
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TRACY_ENABLE "Enable Tracy profiler" ON)
set(TRACY_NO_SYSTEM_TRACING ON CACHE BOOL "Disable system tracing")
# Clang-Tidy
set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -extra-arg=-std=c++23;
    --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
)

# clang-format helper (optional)
add_custom_target(format
    COMMAND clang-format -i $(git ls-files "*.cpp" "*.h")
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
if (MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)
# Build toggles for core only
option(ZEMITA_BUILD_CLI "Build the myzip command-line tool" ON)
option(ZEMITA_INSTALL   "Enable install rules"               ON)

# Core code lives in src/
add_subdirectory(src)
add_subdirectory(tracy)