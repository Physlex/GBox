cmake_minimum_required(VERSION 3.28.3)
project(gbox VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(GBOX_TESTS_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests")

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project-wide dependencies

if(NOT CMAKE_C_COMPILER MATCHES "clang")
  message(
    FATAL_ERROR
    "clang improperly installed. Did you use `nix develop`?"
  )
endif()

find_package(GTest REQUIRED)

include(GoogleTest)
enable_testing()

# Build system configuration

set(GBOX_CLANG_WRAPPER_NAME gbclang)
set(PROJECT_CONF_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(${PROJECT_CONF_PATH}/AddTests.cmake)
include(${PROJECT_CONF_PATH}/TargetFormat.cmake)
include(${PROJECT_CONF_PATH}/AddModule.cmake)
include(${PROJECT_CONF_PATH}/AddLibrary.cmake)

# Workspace module libraries

gbox_module(GBOX_CORE "${PROJECT_SOURCE_DIR}/libs/core")
gbox_module(GBOX_PROC "${PROJECT_SOURCE_DIR}/libs/proc")
gbox_module(GBOX_RUNTIME "${PROJECT_SOURCE_DIR}/libs/runtime")

# Workspace module applications

gbox_module(GBCLANG "${PROJECT_SOURCE_DIR}/apps/gbclang")
gbox_module(TREPIDATION "${PROJECT_SOURCE_DIR}/apps/trepidation")

# Build mode selection. Libraries always build; GBOX_BUILD_TEST decides whether the app
# binaries or the tests come along, and the inactive set is dropped from the `all` target
# so its sources are never scanned or compiled.
if(GBOX_BUILD_TEST)
  get_property(GBOX_INACTIVE_TARGETS GLOBAL PROPERTY GBOX_APPS)
else()
  get_property(GBOX_INACTIVE_TARGETS GLOBAL PROPERTY GBOX_TESTS)
endif()

foreach(TARGET IN LISTS GBOX_INACTIVE_TARGETS)
  set_target_properties("${TARGET}" PROPERTIES EXCLUDE_FROM_ALL ON)
endforeach()
