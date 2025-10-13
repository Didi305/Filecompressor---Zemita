#----------------------------------------------------------------
# Generated CMake target import file for configuration "MinSizeRel".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "zemita::zemita" for configuration "MinSizeRel"
set_property(TARGET zemita::zemita APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(zemita::zemita PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/lib/zemita.lib"
  )

list(APPEND _cmake_import_check_targets zemita::zemita )
list(APPEND _cmake_import_check_files_for_zemita::zemita "${_IMPORT_PREFIX}/lib/zemita.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
