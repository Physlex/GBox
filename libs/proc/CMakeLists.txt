# Set up LLVM and clang dependencies...

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(LLVM_LINK_COMPONENTS Support)

find_package(LLVM REQUIRED CONFIG)
include(AddLLVM)

find_package(Clang REQUIRED CONFIG)
include(${Clang_DIR}/AddClang.cmake)

add_definitions(${LLVM_DEFINITIONS})

add_library(${GBOX_PROC_NAME} STATIC ${GBOX_PROC_SRC})
target_include_directories(${GBOX_PROC_NAME}
  PUBLIC
  ${GBOX_PROC_INC_PATH}
  ${GBOX_CORE_INC_PATH}
  ${CLANG_INCLUDE_DIRS}
  ${LLVM_INCLUDE_DIRS}
)

target_link_libraries(${GBOX_PROC_NAME}
  PUBLIC
  clangAST
  clangTooling
  clangBasic
  clangFrontend
)
