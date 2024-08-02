# Generated by CMake

if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.8)
   message(FATAL_ERROR "CMake >= 2.8.0 required")
endif()
if(CMAKE_VERSION VERSION_LESS "3.0.0")
   message(FATAL_ERROR "CMake >= 3.0.0 required")
endif()
cmake_policy(PUSH)
cmake_policy(VERSION 3.0.0...3.27)
#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Protect against multiple inclusion, which would fail when already imported targets are added once more.
set(_cmake_targets_defined "")
set(_cmake_targets_not_defined "")
set(_cmake_expected_targets "")
foreach(_cmake_expected_target IN ITEMS AMReX::amrex_3d AMReX::Flags_CXX AMReX::Flags_INLINE AMReX::Flags_Fortran AMReX::Flags_FPE)
  list(APPEND _cmake_expected_targets "${_cmake_expected_target}")
  if(TARGET "${_cmake_expected_target}")
    list(APPEND _cmake_targets_defined "${_cmake_expected_target}")
  else()
    list(APPEND _cmake_targets_not_defined "${_cmake_expected_target}")
  endif()
endforeach()
unset(_cmake_expected_target)
if(_cmake_targets_defined STREQUAL _cmake_expected_targets)
  unset(_cmake_targets_defined)
  unset(_cmake_targets_not_defined)
  unset(_cmake_expected_targets)
  unset(CMAKE_IMPORT_FILE_VERSION)
  cmake_policy(POP)
  return()
endif()
if(NOT _cmake_targets_defined STREQUAL "")
  string(REPLACE ";" ", " _cmake_targets_defined_text "${_cmake_targets_defined}")
  string(REPLACE ";" ", " _cmake_targets_not_defined_text "${_cmake_targets_not_defined}")
  message(FATAL_ERROR "Some (but not all) targets in this export set were already defined.\nTargets Defined: ${_cmake_targets_defined_text}\nTargets not yet defined: ${_cmake_targets_not_defined_text}\n")
endif()
unset(_cmake_targets_defined)
unset(_cmake_targets_not_defined)
unset(_cmake_expected_targets)


# Compute the installation prefix relative to this file.
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
if(_IMPORT_PREFIX STREQUAL "/")
  set(_IMPORT_PREFIX "")
endif()

# Create imported target AMReX::amrex_3d
add_library(AMReX::amrex_3d STATIC IMPORTED)

set_target_properties(AMReX::amrex_3d PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "AMREX_SPACEDIM=3"
  INTERFACE_COMPILE_FEATURES "cxx_std_17"
  INTERFACE_COMPILE_OPTIONS "\$<\$<CXX_COMPILER_ID:Cray>:-h;noomp>;\$<\$<AND:\$<COMPILE_LANGUAGE:CXX>,\$<CXX_COMPILER_ID:MSVC>>:\$<IF:\$<VERSION_LESS:\$<CXX_COMPILER_VERSION>,19.26>,/experimental:preprocessor,/Zc:preprocessor>>;\$<\$<AND:\$<COMPILE_LANGUAGE:CXX>,\$<CXX_COMPILER_ID:MSVC>>:\$<\$<VERSION_GREATER_EQUAL:\$<CXX_COMPILER_VERSION>,19.14>:/Zc:__cplusplus>>;\$<\$<CXX_COMPILER_ID:Cray>:-h;noomp>"
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
  INTERFACE_LINK_LIBRARIES "Threads::Threads"
)

# Create imported target AMReX::Flags_CXX
add_library(AMReX::Flags_CXX INTERFACE IMPORTED)

set_target_properties(AMReX::Flags_CXX PROPERTIES
  INTERFACE_COMPILE_OPTIONS "\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,GNU>,\$<CONFIG:Debug>>:-O0;-ggdb;-Wall;-Wno-sign-compare;-Wno-unused-but-set-variable;-Werror=return-type>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,GNU>,\$<CONFIG:RelWithDebInfo>>:-Werror=return-type>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,GNU>,\$<CONFIG:Release>>:-Werror=return-type>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,Intel>,\$<CONFIG:Debug>>:-O0;-traceback;-Wcheck>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,Intel>,\$<CONFIG:RelWithDebInfo>>:-ip;-qopt-report=5;-qopt-report-phase=vec>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,Intel>,\$<CONFIG:Release>>:-ip;-qopt-report=5;-qopt-report-phase=vec>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,PGI>,\$<CONFIG:Debug>>:-O0;-Mbounds>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,PGI>,\$<CONFIG:RelWithDebInfo>>:-gopt;-fast>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,PGI>,\$<CONFIG:Release>>:-gopt;-fast>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,Cray>,\$<CONFIG:Debug>>:-O0>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,Cray>,\$<CONFIG:RelWithDebInfo>>:>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,Cray>,\$<CONFIG:Release>>:>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,Clang>,\$<CONFIG:Debug>>:-O0;-Wall;-Wextra;-Wno-sign-compare;-Wno-unused-parameter;-Wno-unused-variable>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,Clang>,\$<CONFIG:RelWithDebInfo>>:>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,Clang>,\$<CONFIG:Release>>:>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,AppleClang>,\$<CONFIG:Debug>>:-O0;-Wall;-Wextra;-Wno-sign-compare;-Wno-unused-parameter;-Wno-unused-variable>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,AppleClang>,\$<CONFIG:RelWithDebInfo>>:>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,AppleClang>,\$<CONFIG:Release>>:>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,IntelLLVM>,\$<CONFIG:Debug>>:-O0;-Wall;-Wextra;-Wno-sign-compare;-Wno-unused-parameter;-Wno-unused-variable>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,IntelLLVM>,\$<CONFIG:RelWithDebInfo>>:-gline-tables-only;-fdebug-info-for-profiling>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:CXX,IntelLLVM>,\$<CONFIG:Release>>:>"
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
)

# Create imported target AMReX::Flags_INLINE
add_library(AMReX::Flags_INLINE INTERFACE IMPORTED)

set_target_properties(AMReX::Flags_INLINE PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
)

# Create imported target AMReX::Flags_Fortran
add_library(AMReX::Flags_Fortran INTERFACE IMPORTED)

set_target_properties(AMReX::Flags_Fortran PROPERTIES
  INTERFACE_COMPILE_OPTIONS "\$<\$<AND:\$<COMPILE_LANG_AND_ID:Fortran,GNU>,\$<CONFIG:Debug>>:-O0;-ggdb;-fcheck=bounds;-fbacktrace;-Wuninitialized;-Wunused;-finit-real=snan;-finit-integer=2147483647;-fimplicit-none>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:Fortran,GNU>,\$<CONFIG:Release>>:-fimplicit-none>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:Fortran,Intel>,\$<CONFIG:Debug>>:-O0;-traceback;-check;bounds,uninit,pointers;-implicitnone>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:Fortran,Intel>,\$<CONFIG:Release>>:-ip;-qopt-report=5;-qopt-report-phase=vec;-implicitnone>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:Fortran,PGI>,\$<CONFIG:Debug>>:-O0;-Mbounds>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:Fortran,PGI>,\$<CONFIG:Release>>:-gopt;-fast>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:Fortran,Cray>,\$<CONFIG:Debug>>:-O0;-e;i>;\$<\$<AND:\$<COMPILE_LANG_AND_ID:Fortran,Cray>,\$<CONFIG:Release>>:>"
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
)

# Create imported target AMReX::Flags_FPE
add_library(AMReX::Flags_FPE INTERFACE IMPORTED)

set_target_properties(AMReX::Flags_FPE PROPERTIES
  INTERFACE_COMPILE_OPTIONS "\$<\$<COMPILE_LANG_AND_ID:Fortran,GNU>:-ffpe-trap=invalid,zero;-ftrapv>;\$<\$<COMPILE_LANG_AND_ID:CXX,GNU>:-ftrapv>;\$<\$<COMPILE_LANG_AND_ID:Fortran,Intel>:-fpe3>;\$<\$<COMPILE_LANG_AND_ID:CXX,Intel>:-fpe3>;\$<\$<COMPILE_LANG_AND_ID:Fortran,PGI>:-Ktrap=divz,inv>;\$<\$<COMPILE_LANG_AND_ID:CXX,PGI>:>;\$<\$<COMPILE_LANG_AND_ID:Fortran,Cray>:-K;trap=fp>;\$<\$<COMPILE_LANG_AND_ID:CXX,Cray>:-K;trap=fp>;\$<\$<COMPILE_LANG_AND_ID:Fortran,Clang>:>;\$<\$<COMPILE_LANG_AND_ID:CXX,Clang>:-ftrapv>;\$<\$<COMPILE_LANG_AND_ID:CXX,AppleClang>:-ftrapv>"
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
)

# Load information for each installed configuration.
file(GLOB _cmake_config_files "${CMAKE_CURRENT_LIST_DIR}/AMReXTargets-*.cmake")
foreach(_cmake_config_file IN LISTS _cmake_config_files)
  include("${_cmake_config_file}")
endforeach()
unset(_cmake_config_file)
unset(_cmake_config_files)

# Cleanup temporary variables.
set(_IMPORT_PREFIX)

# Loop over all imported files and verify that they actually exist
foreach(_cmake_target IN LISTS _cmake_import_check_targets)
  if(CMAKE_VERSION VERSION_LESS "3.28"
      OR NOT DEFINED _cmake_import_check_xcframework_for_${_cmake_target}
      OR NOT IS_DIRECTORY "${_cmake_import_check_xcframework_for_${_cmake_target}}")
    foreach(_cmake_file IN LISTS "_cmake_import_check_files_for_${_cmake_target}")
      if(NOT EXISTS "${_cmake_file}")
        message(FATAL_ERROR "The imported target \"${_cmake_target}\" references the file
   \"${_cmake_file}\"
but this file does not exist.  Possible reasons include:
* The file was deleted, renamed, or moved to another location.
* An install or uninstall procedure did not complete successfully.
* The installation package was faulty and contained
   \"${CMAKE_CURRENT_LIST_FILE}\"
but not all the files it references.
")
      endif()
    endforeach()
  endif()
  unset(_cmake_file)
  unset("_cmake_import_check_files_for_${_cmake_target}")
endforeach()
unset(_cmake_target)
unset(_cmake_import_check_targets)

# This file does not depend on other imported targets which have
# been exported from the same project but in a separate export set.

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
cmake_policy(POP)
