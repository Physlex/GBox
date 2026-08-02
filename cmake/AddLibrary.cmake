# AddLibrary.cmake
#
# This file declares gbox modules as library and binary targets.
#
# Every dependency in the workspace is a module: one built in this tree is `gbox::<name>`, and
# one brought in from a third-party package is `import::<name>`. `gbox_dependencies()` takes a
# single uniform list of bare names and resolves each against both namespaces, so call sites do
# not distinguish between the two.
#
# `gbox_module()` (AddModule.cmake) discovers a directory and globs its sources; the commands
# here turn what it discovered into targets, and must be called from the module's own
# CMakeLists.txt so that the `<MODULE>_*` variables are in scope.
#
# A module's CMakeLists.txt reads as its targets followed by what those targets need:
#
#     gbox_binary(trepidation)
#     gbox_import(sdl3 PACKAGE SDL3 COMPONENTS SDL3-shared LINK SDL3::SDL3-shared)
#     gbox_dependencies(runtime core)
#
# A directory may declare several libraries and binaries. Dependencies are declared for the
# directory rather than for one target, so every target in the file receives them.

# Resolves each bare module name against the `gbox::` and `import::` namespaces and links the
# result to TARGET at the given SCOPE. Errors if a name matches neither namespace.
function(gbox_link_dependencies TARGET SCOPE)
    foreach(DEPENDENCY IN LISTS ARGN)
        if(TARGET "gbox::${DEPENDENCY}")
            set(RESOLVED "gbox::${DEPENDENCY}")
        elseif(TARGET "import::${DEPENDENCY}")
            set(RESOLVED "import::${DEPENDENCY}")
        else()
            message(
              FATAL_ERROR
              "gbox link ${TARGET}: no module named `${DEPENDENCY}`, declare it with "
              "gbox_library or gbox_import before this call."
            )
        endif()

        target_link_libraries("${TARGET}" ${SCOPE} "${RESOLVED}")
    endforeach()
endfunction()

# Declares an in-tree module as a gbox library, exported downstream as `gbox::<name>` and
# reachable as the `<name>` component of the gbox package.
#
#   INCLUDES   <dir>..     include directories in addition to the module's own inc/
#   NO_MODULES             opt out of C++20 module scanning for this library
function(gbox_library LIB_NAME)
    cmake_parse_arguments(GBLIB "NO_MODULES" "" "INCLUDES" ${ARGN})

    set(MODULE "${GBOX_CURRENT_MODULE}")
    set(TARGET "gbox_${LIB_NAME}")

    add_library("${TARGET}")
    add_library("gbox::${LIB_NAME}" ALIAS "${TARGET}")

    target_sources("${TARGET}" PRIVATE ${${MODULE}_SRC})

    if(GBLIB_NO_MODULES)
        # A target property rather than the directory-scope variable, so opting out here cannot
        # affect anything else configured from the same directory.
        set_target_properties("${TARGET}" PROPERTIES CXX_SCAN_FOR_MODULES OFF)
    else()
        target_sources(
            "${TARGET}"
            PUBLIC
            FILE_SET CXX_MODULES
            BASE_DIRS "${${MODULE}_SRC_PATH}"
            FILES ${${MODULE}_MODULE_SRC}
        )
    endif()

    # Public headers are declared as a file set rather than a bare include directory. Both
    # behave identically in-tree, but only a file set carries the information needed to install
    # the headers alongside the target.
    if(EXISTS "${${MODULE}_INC_PATH}")
        file(
            GLOB_RECURSE
            "${MODULE}_HEADERS"
            "${${MODULE}_INC_PATH}/*.h" "${${MODULE}_INC_PATH}/*.hpp"
        )

        target_sources(
            "${TARGET}"
            PUBLIC
            FILE_SET HEADERS
            BASE_DIRS "${${MODULE}_INC_PATH}"
            FILES ${${MODULE}_HEADERS}
        )
    endif()

    target_include_directories("${TARGET}" PUBLIC ${GBLIB_INCLUDES})

    target_compile_features("${TARGET}" PUBLIC cxx_std_23)

    # The component a library is reachable under downstream. Carried on the target so that
    # gbox_module() can read it back without the name being derived a second time.
    #
    # EXPORT_NAME is what an export set namespaces, and it defaults to the target's real name
    # rather than its alias -- without it a consumer would be handed `gbox::gbox_core`.
    set_target_properties("${TARGET}" PROPERTIES
        GBOX_COMPONENT "${LIB_NAME}"
        EXPORT_NAME "${LIB_NAME}"
    )

    set_property(GLOBAL APPEND PROPERTY GBOX_LIBRARIES "${TARGET}")
    set_property(DIRECTORY APPEND PROPERTY GBOX_DIR_LIBRARIES "${TARGET}")
endfunction()

# Declares an in-tree module as a gbox binary. The module's `src/main.cpp` is the entry point;
# the rest of its sources (which gbox_module() already strips of main.*) come along so a binary
# may keep private helpers beside main.
#
# Binaries are installed as programs but never exported: nothing links a binary, so it carries no
# imported target and belongs to no component.
#
#   INCLUDES   <dir>..     include directories in addition to the module's own inc/
#   NO_MODULES             opt out of C++20 module scanning for this binary
function(gbox_binary APP_NAME)
    cmake_parse_arguments(GBAPP "NO_MODULES" "" "INCLUDES" ${ARGN})

    set(MODULE "${GBOX_CURRENT_MODULE}")
    set(TARGET "${APP_NAME}")

    add_executable("${TARGET}" "${${MODULE}_SRC_PATH}/main.cpp" ${${MODULE}_SRC})

    if(GBAPP_NO_MODULES)
        set_target_properties("${TARGET}" PROPERTIES CXX_SCAN_FOR_MODULES OFF)
    endif()

    target_include_directories("${TARGET}" PRIVATE ${GBAPP_INCLUDES})

    set_property(GLOBAL APPEND PROPERTY GBOX_APPS "${TARGET}")
    set_property(DIRECTORY APPEND PROPERTY GBOX_DIR_BINARIES "${TARGET}")
endfunction()

# Declares the modules that this directory's targets link, by short name. Every library and
# binary declared in the file receives them, so a directory states its dependencies once no
# matter how many targets it builds.
#
# Libraries take them PUBLIC: a module interface unit that re-exports a dependency's
# declarations makes that dependency part of the library's own interface. Binaries take them
# PRIVATE, having no interface of their own.
function(gbox_dependencies)
    get_property(LIBRARIES DIRECTORY PROPERTY GBOX_DIR_LIBRARIES)
    get_property(BINARIES DIRECTORY PROPERTY GBOX_DIR_BINARIES)

    if(NOT LIBRARIES AND NOT BINARIES)
        message(
          FATAL_ERROR
          "gbox link ${GBOX_CURRENT_MODULE}: no library or binary declared in this directory, "
          "call `gbox_library` or `gbox_binary` before declaring dependencies."
        )
    endif()

    foreach(TARGET IN LISTS LIBRARIES)
        gbox_link_dependencies("${TARGET}" PUBLIC ${ARGN})
    endforeach()

    foreach(TARGET IN LISTS BINARIES)
        gbox_link_dependencies("${TARGET}" PRIVATE ${ARGN})
    endforeach()

    set_property(DIRECTORY APPEND PROPERTY GBOX_DIR_DEPENDENCIES ${ARGN})
endfunction()

# Declares a third-party package as a gbox module.
#
#   PACKAGE    <pkg>      configuration package to locate, i.e. find_package(<pkg> REQUIRED CONFIG)
#   COMPONENTS <comp>..   components to request from PACKAGE
#   LINK       <target>.. targets the package provides that dependents must link
#   INCLUDES   <dir>..    additional include directories
#
# The include directories and compile definitions a package advertises through the conventional
# <PKG>_INCLUDE_DIRS and <PKG>_DEFINITIONS variables are picked up automatically, so a package
# following that convention needs only its PACKAGE and LINK entries.
#
# Importing a package also declares it a dependency of this directory, since an import is not
# worth resolving unless something in the directory links it.
function(gbox_import IMPORT_NAME)
    cmake_parse_arguments(GBIMPORT "" "PACKAGE" "LINK;INCLUDES;COMPONENTS" ${ARGN})

    set(TARGET "import_${IMPORT_NAME}")
    set(INCLUDES ${GBIMPORT_INCLUDES})
    set(DEFINITIONS "")

    if(GBIMPORT_COMPONENTS AND NOT DEFINED GBIMPORT_PACKAGE)
        message(
          FATAL_ERROR
          "gbox import ${IMPORT_NAME}: COMPONENTS given without PACKAGE, components can only "
          "be requested from a package."
        )
    endif()

    if(DEFINED GBIMPORT_PACKAGE)
        # Imported targets are scoped to the directory that found them, which would put them
        # out of reach of sibling directories -- test directories in particular, since
        # gbox_module() adds those alongside the module rather than beneath it.
        set(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL TRUE)

        set(FIND_ARGS "${GBIMPORT_PACKAGE}" REQUIRED CONFIG)
        if(GBIMPORT_COMPONENTS)
            list(APPEND FIND_ARGS COMPONENTS ${GBIMPORT_COMPONENTS})
        endif()
        find_package(${FIND_ARGS})

        # A package exports these under its own name or under an upper-cased one; LLVM uses the
        # former and Clang the latter, so accept whichever is populated.
        string(TOUPPER "${GBIMPORT_PACKAGE}" PACKAGE_UPPER)
        foreach(PREFIX IN ITEMS "${GBIMPORT_PACKAGE}" "${PACKAGE_UPPER}")
            list(APPEND INCLUDES ${${PREFIX}_INCLUDE_DIRS})
            list(APPEND DEFINITIONS ${${PREFIX}_DEFINITIONS})
        endforeach()

        list(REMOVE_DUPLICATES INCLUDES)
        list(REMOVE_DUPLICATES DEFINITIONS)
    endif()

    add_library("${TARGET}" INTERFACE)
    add_library("import::${IMPORT_NAME}" ALIAS "${TARGET}")

    # Exported alongside whichever component links it, where the package namespace applies. An
    # import therefore arrives downstream as `gbox::<name>` rather than `import::<name>`.
    set_target_properties("${TARGET}" PROPERTIES EXPORT_NAME "${IMPORT_NAME}")

    target_link_libraries("${TARGET}" INTERFACE ${GBIMPORT_LINK})
    target_compile_definitions("${TARGET}" INTERFACE ${DEFINITIONS})

    # SYSTEM keeps third-party headers from raising warnings the workspace cannot fix.
    target_include_directories("${TARGET}" SYSTEM INTERFACE ${INCLUDES})

    set_property(GLOBAL APPEND PROPERTY GBOX_IMPORTS "${IMPORT_NAME}")
    set_property(GLOBAL PROPERTY "GBOX_IMPORT_${IMPORT_NAME}_PACKAGE" "${GBIMPORT_PACKAGE}")
    set_property(
      GLOBAL PROPERTY "GBOX_IMPORT_${IMPORT_NAME}_COMPONENTS" "${GBIMPORT_COMPONENTS}"
    )

    gbox_dependencies("${IMPORT_NAME}")
endfunction()
