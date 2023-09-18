cmake_minimum_required(VERSION 3.16)

option(CMAKE_VERBOSE_MAKEFILE "Create verbose makefile" OFF)
option(USE_NSIS "Use NSIS to create installer" OFF)

if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET})
  message(STATUS "Setting default vcpkg target triplet to $ENV{VCPKG_DEFAULT_TRIPLET}")
  set(VCPKG_TARGET_TRIPLET $ENV{VCPKG_DEFAULT_TRIPLET})
endif()

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
  set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)
  message(STATUS "VCPKG found: $ENV{VCPKG_ROOT}")
  message(STATUS "Using VCPKG integration")
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
  if(VCPKG_MANIFEST_FEATURES)
    message(STATUS "VCPKG_MANIFEST_FEATURES: ${VCPKG_MANIFEST_FEATURES}")
  endif()
elseif(DEFINED CMAKE_TOOLCHAIN_FILE)
  message(STATUS "Using toolchain: ${CMAKE_TOOLCHAIN_FILE}")
  if(CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg.cmake")
    message(STATUS "Toolchain uses VCPKG integration")
    if(VCPKG_MANIFEST_FEATURES)
      message(STATUS "VCPKG_MANIFEST_FEATURES: ${VCPKG_MANIFEST_FEATURES}")
    endif()
  endif()
else()
  message(WARNING "vcpkg not found, toolchain not defined, build might fail")
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json)
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json VCPKG_JSON_STRING)
  string(JSON VERSION_STRING GET ${VCPKG_JSON_STRING} version)
else()
  set(VERSION_STRING "0.0.0")
endif()

string(REPLACE "." ";" VERSION_LIST ${VERSION_STRING})
list(LENGTH VERSION_LIST VERSION_LIST_LENGTH)
if(VERSION_LIST_LENGTH LESS 2)
  message(FATAL_ERROR "Tool needs at least major.minor version numbers to properly configure")
endif()
list(GET VERSION_LIST 0 TOOL_MAJOR_VERSION)
list(GET VERSION_LIST 1 TOOL_MINOR_VERSION)
if(VERSION_LIST_LENGTH GREATER 2)
  list(GET VERSION_LIST 2 TOOL_PATCH_VERSION)
else()
  set(TOOL_PATCH_VERSION 0)
endif()
if(VERSION_LIST_LENGTH GREATER 3)
  list(GET VERSION_LIST 3 TOOL_TWEAK_VERSION)
else()
  set(TOOL_TWEAK_VERSION 0)
endif()

set(TOOL_VERSION ${TOOL_MAJOR_VERSION}.${TOOL_MINOR_VERSION}.${TOOL_PATCH_VERSION}.${TOOL_TWEAK_VERSION})
message(STATUS "TOOL_VERSION: ${TOOL_VERSION}")

project(udp_simple_server VERSION ${TOOL_VERSION})

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "Install prefix" FORCE)
endif()

set(INSTALL_BIN_DIR      "bin"                   CACHE PATH "Path where exe and dll will be installed")
set(INSTALL_LIB_DIR      "lib"                   CACHE PATH "Path where lib will be installed")
set(INSTALL_INCLUDE_DIR  "include"               CACHE PATH "Path where headers will be installed ")
set(INSTALL_CMAKE_DIR    "share/${PROJECT_NAME}" CACHE PATH "Path where cmake configs will be installed")

find_package(Boost COMPONENTS date_time filesystem regex serialization system REQUIRED)

add_executable(${CMAKE_PROJECT_NAME}
  "${CMAKE_CURRENT_SOURCE_DIR}/src/udp_simple_ser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/udp_lib.hpp"
  "${CMAKE_SOURCE_DIR}/src/md5.cpp"
  "${CMAKE_SOURCE_DIR}/src/md5.h"
)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 11)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>)
target_link_libraries(${CMAKE_PROJECT_NAME}
  Boost::boost
  Boost::filesystem
  Boost::serialization
  Boost::system
)

install(TARGETS ${CMAKE_PROJECT_NAME}
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
)

set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
include(InstallRequiredSystemLibraries)

install(
    PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION ${INSTALL_BIN_DIR}
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json)
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json VCPKG_JSON_STRING)
  string(JSON CPACK_PACKAGE_NAME GET ${VCPKG_JSON_STRING} name)
  string(JSON CPACK_PACKAGE_HOMEPAGE_URL GET ${VCPKG_JSON_STRING} homepage)
  string(JSON CPACK_PACKAGE_DESCRIPTION GET ${VCPKG_JSON_STRING} description)
  #set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)

  if(UNIX AND NOT APPLE)
    find_program(LSB_RELEASE_EXEC lsb_release)
    execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
        OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(LSB_RELEASE_ID_SHORT STREQUAL "Ubuntu")
      set(CPACK_GENERATOR "DEB")
      set(CPACK_DEBIAN_PACKAGE_MAINTAINER "cenit")
    else()
      set(CPACK_GENERATOR "RPM")
    endif()
  elseif(APPLE)
      set(CPACK_GENERATOR "DragNDrop")
  elseif(WIN32)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
    if(USE_NSIS)
      set(CPACK_GENERATOR "NSIS")
      string(JSON CPACK_NSIS_PACKAGE_NAME GET ${VCPKG_JSON_STRING} name)
      string(JSON CPACK_NSIS_DISPLAY_NAME GET ${VCPKG_JSON_STRING} name)
      set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
      set(CPACK_NSIS_MODIFY_PATH OFF) #disable extra page for adding to PATH, because it's broken on Win10+ due to NSIS not supporting MAX_PATH
    else()
      set(CPACK_GENERATOR "WIX")
      set(CPACK_WIX_UPGRADE_GUID "168BCC41-3878-4539-ADAB-5223E8DBA296") # IMPORTANT! It has to be unique for every project!!
    endif()
  endif()

  include(CPack)
endif()
