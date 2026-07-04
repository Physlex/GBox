cmake_minimum_required(VERSION 3.28.3)
project(firmware VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

if(NOT CMAKE_C_COMPILER MATCHES "clang")
    message(
      FATAL_ERROR
      "clang improperly installed. Are you using the nix shell?"
    )
endif()

add_executable(${PROJECT_NAME} src/main.cpp)

add_subdirectory(gen)

target_link_libraries(${PROJECT_NAME} PRIVATE
  STM32_Drivers
  stm32cubemx
  ${GCC_MULTILIB_DIR_GCC}/crti.o
  ${GCC_MULTILIB_DIR_ARM}/libc.a
  ${GCC_MULTILIB_DIR_ARM}/libm.a
  ${GCC_MULTILIB_DIR_GCC}/libgcc.a
  ${GCC_MULTILIB_DIR_ARM}/libnosys.a
  ${GCC_MULTILIB_DIR_GCC}/crtn.o
)

target_link_options(${PROJECT_NAME} PRIVATE
  -T ${CMAKE_SOURCE_DIR}/gen/STM32F446XX_FLASH.ld
  -Wl,--gc-sections
  -Wl,-Map=${PROJECT_NAME}.map
  -Wl,--print-memory-usage
  -Wl,-L${GCC_MULTILIB_DIR_ARM}
  -Wl,-L${GCC_MULTILIB_DIR_GCC}
  -z noexecstack
)
