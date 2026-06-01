# This file implements a method of marking a directory as a module
# 
# A module is any set of src,inc, and test code that makes up a "member" of the overall
# project.
# 

# TODO: DOCS
macro(gbox_module MODULE_NAME MODULE_PATH)
  set("${MODULE_NAME}_PATH" "${MODULE_PATH}")
  string(TOLOWER "${MODULE_NAME}" "${MODULE_NAME}_LIB_NAME")

  set("${MODULE_NAME}_NAME" "${${MODULE_NAME}_LIB_NAME}")
  set("${MODULE_NAME}_SRC_PATH" "${MODULE_PATH}/src")
  set("${MODULE_NAME}_INC_PATH" "${MODULE_PATH}/inc")
  set("${MODULE_NAME}_TEST_PATH" "${MODULE_PATH}/test")

  file(
    GLOB_RECURSE
    "${MODULE_NAME}_SRC"
    "${${MODULE_NAME}_SRC_PATH}/*.cpp" "${${MODULE_NAME}_SRC_PATH}/*.c"
  )

  list(REMOVE_ITEM "${MODULE_NAME}_SRC" "${${MODULE_NAME}_SRC_PATH}/main.cpp")
  list(REMOVE_ITEM "${MODULE_NAME}_SRC" "${${MODULE_NAME}_SRC_PATH}/main.c")

  add_subdirectory("${${MODULE_NAME}_PATH}")
  if(EXISTS "${${MODULE_NAME}_TEST_PATH}/CMakeLists.txt")
    add_subdirectory("${${MODULE_NAME}_TEST_PATH}")
  else()
    message(STATUS "gbox_module(${MODULE_NAME}): no test directory found at ${${MODULE_NAME}_TEST_PATH}")
  endif()
endmacro()
