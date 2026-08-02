file(GLOB_RECURSE TIDY_FILES *.c *.cpp *.cppm)
add_custom_target(tidy COMMAND clang-tidy -p ${PROJECT_BINARY_DIR} --quiet ${TIDY_FILES})
