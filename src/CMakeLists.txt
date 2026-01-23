# ---- Library (core) ----
cmake_minimum_required(VERSION 4.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_library(zemita
  zemita.cpp
  container/container.cpp
  container/index.cpp
  io/buffered_reader.cpp
  io/buffered_writer.cpp
  codecs/lz77.cpp
)
target_link_libraries(zemita PUBLIC Tracy::TracyClient)
include(GNUInstallDirs)
add_library(zemita::zemita ALIAS zemita)


target_include_directories(zemita
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/unordered_dense/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(zemita PUBLIC cxx_std_20)

# ---- CLI (optional) ----
if(ZEMITA_BUILD_CLI)
  add_executable(zemita_cli CLI/zemita_main.cpp)
  target_link_libraries(zemita_cli PRIVATE zemita)
  set_target_properties(zemita_cli PROPERTIES OUTPUT_NAME zemita)
endif()

# ---- Install (optional) ----
if(ZEMITA_INSTALL)
  include(GNUInstallDirs)

  install(TARGETS zemita
          EXPORT zemitaTargets
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

  if(zemita_BUILD_CLI)
    install(TARGETS zemita_cli
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()

  install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/zemita
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  install(EXPORT zemitaTargets
          NAMESPACE zemita::
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/zemita)
endif()