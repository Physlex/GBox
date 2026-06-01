cmake_minimum_required(VERSION 3.28.3)
project(stm32f4_uart VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

if(NOT CMAKE_C_COMPILER MATCHES "clang")
    message(
      FATAL_ERROR
      "clang improperly installed. Did you use source the nix shell using `nix develop`?"
    )
endif()

find_package(gbox REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)

message("TEST: ${PROJECT_SOURCE_DIR}")
gbox_module(HAL "${PROJECT_SOURCE_DIR}/gen")

target_link_libraries(${PROJECT_NAME} PRIVATE
  gbox::runtime
  gbox::core
  STM32_Drivers
  stm32cubemx
)
