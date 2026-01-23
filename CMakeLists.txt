cmake_minimum_required(VERSION 4.0)

project(Zemita VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Copy compile_commands.json to root for clangd/clang-tidy
if(EXISTS "${CMAKE_BINARY_DIR}/compile_commands.json")
    configure_file(
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
        COPYONLY
    )
endif()


# Fetch Tracy


# Clang-Tidy
set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -extra-arg=-std=c++23;
    --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
)

if (MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)

option(ZEMITA_BUILD_CLI "Build the myzip command-line tool" ON)
option(ZEMITA_INSTALL   "Enable install rules" ON)

set(TRACY_ENABLE ON CACHE BOOL "Enable Tracy profiler")
set(TRACY_NO_SYSTEM_TRACING ON CACHE BOOL "Disable system tracing")

# Add Tracy
add_subdirectory(external/tracy)
add_subdirectory(src)