cmake_minimum_required(VERSION 3.28.3)
project(app-loop VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

if(
  NOT CMAKE_C_COMPILER MATCHES "clang" OR
  NOT CMAKE_CXX_COMPILER MATCHES "clang\\+\\+"
  )
    message(
      FATAL_ERROR
      "clang improperly installed. Did you use `nix develop`?"
    )
endif()

find_package(gbox REQUIRED COMPONENTS runtime)

gbox_module(APP_LOOP "${CMAKE_CURRENT_SOURCE_DIR}/app")
