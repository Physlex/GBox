cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

enable_language(C ASM)

add_library("${HAL_CORE_NAME}" STATIC "${HAL_SRC}")

add_subdirectory(cmake/stm32cubemx)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_link_libraries(${PROJECT_NAME} PRIVATE stm32cubemx)
