# AddTests.cmake
#
# This file converts a set of globbed source files into tests binaries.

function(add_tests)
    # Parse keyword arguments:
    #   SOURCES   - list of source files
    #   INCLUDES  - list of include dirs
    #   LIBRARIES - list of libraries
    cmake_parse_arguments(
        ADDTEST
        ""  # no options
        ""  # no single-value args
        "SOURCES;INCLUDES;LIBRARIES"  # multi-value args
        ${ARGN}
    )

    # The nix clang wrapper injects GTest's include dir (via NIX_CFLAGS_COMPILE), so CMake
    # records it as an implicit system include and strips it back out of the -I flags. But
    # clang-scan-deps (C++ module scanning) does not run through the wrapper, so it never
    # gets that implicit injection either -- a test that `import`s a module then fails to
    # resolve <gtest/gtest.h> while being scanned. Force the dir onto the command line as a
    # raw -isystem compile option (which CMake does not strip) so the scanner can find it.
    # GTest is a REQUIRED package, so GTest_DIR is always set and its layout is fixed.
    # FIXME: Move out of cmake once gbox build system has formalized method of test harness choosing
    cmake_path(SET GTEST_INC_DIR NORMALIZE "${GTest_DIR}/../../../include")

    foreach(TEST IN LISTS ADDTEST_SOURCES)
        cmake_path(GET TEST STEM TEST_NAME)
        set(RUNTIME_NAME "test-${TEST_NAME}")

        add_executable(${RUNTIME_NAME} ${TEST})

        target_include_directories(${RUNTIME_NAME} PRIVATE ${ADDTEST_INCLUDES})
        target_compile_options(${RUNTIME_NAME} PRIVATE "SHELL:-isystem ${GTEST_INC_DIR}")

        target_link_libraries(
            ${RUNTIME_NAME} PRIVATE ${ADDTEST_LIBRARIES} GTest::gtest GTest::gtest_main
        )

        add_test(NAME ${RUNTIME_NAME} COMMAND ${RUNTIME_NAME})
    endforeach()
endfunction()
