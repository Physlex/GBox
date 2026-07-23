# AddLibrary.cmake
#
# This file declares gbox modules as library targets.
#
# Every dependency in the workspace is a module: one built in this tree is `gbox::<name>`, and
# one brought in from a third-party package is `import::<name>`. `gbox_library(... DEPENDS ...)`
# takes a single uniform list of bare names and resolves each against both namespaces, so call
# sites do not distinguish between the two.
#
# `gbox_module()` (AddModule.cmake) discovers a directory and globs its sources; the functions
# here turn what it discovered into targets, and must be called from the module's own
# CMakeLists.txt so that the `<MODULE>_*` variables are in scope.

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
function(gbox_import IMPORT_NAME)
    cmake_parse_arguments(GBIMPORT "" "PACKAGE" "LINK;INCLUDES;COMPONENTS" ${ARGN})

    set(TARGET "import_${IMPORT_NAME}")
    set(INCLUDES ${GBIMPORT_INCLUDES})
    set(DEFINITIONS "")

    if(GBIMPORT_COMPONENTS AND NOT DEFINED GBIMPORT_PACKAGE)
        message(FATAL_ERROR "gbox_import(${IMPORT_NAME}): COMPONENTS requires PACKAGE.")
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

    target_link_libraries("${TARGET}" INTERFACE ${GBIMPORT_LINK})
    target_compile_definitions("${TARGET}" INTERFACE ${DEFINITIONS})

    # SYSTEM keeps third-party headers from raising warnings the workspace cannot fix.
    target_include_directories("${TARGET}" SYSTEM INTERFACE ${INCLUDES})

    set_property(GLOBAL APPEND PROPERTY GBOX_IMPORTS "${IMPORT_NAME}")
    set_property(GLOBAL PROPERTY "GBOX_IMPORT_${IMPORT_NAME}_PACKAGE" "${GBIMPORT_PACKAGE}")
    set_property(
      GLOBAL PROPERTY "GBOX_IMPORT_${IMPORT_NAME}_COMPONENTS" "${GBIMPORT_COMPONENTS}"
    )
endfunction()

# Declares an in-tree module as a gbox library.
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
              "gbox: no module named `${DEPENDENCY}`. Declare it with gbox_module()/"
              "gbox_library() or gbox_import() before this call."
            )
        endif()

        target_link_libraries("${TARGET}" ${SCOPE} "${RESOLVED}")
    endforeach()
endfunction()

#
#   DEPENDS    <module>..  other gbox modules, in-tree or imported, by short name
#   INCLUDES   <dir>..     include directories in addition to the module's own inc/
#   NO_MODULES             opt out of C++20 module scanning for this library
function(gbox_library LIB_NAME)
    cmake_parse_arguments(GBLIB "NO_MODULES" "" "DEPENDS;INCLUDES" ${ARGN})

    # TODO: Read the name the module was registered under directly, once gbox_module() makes
    # MODULE_NAME available to the subdirectory it configures.
    string(TOUPPER "${LIB_NAME}" LIB_NAME_UPPER)
    set(MODULE "GBOX_${LIB_NAME_UPPER}")
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

    # PUBLIC: a module interface unit that re-exports a dependency's declarations makes that
    # dependency part of this library's own interface.
    gbox_link_dependencies("${TARGET}" PUBLIC ${GBLIB_DEPENDS})

    set_property(GLOBAL APPEND PROPERTY GBOX_LIBRARIES "${TARGET}")
endfunction()

# Declares an in-tree module as a gbox application executable. The module's `src/main.cpp` is
# the entry point; the rest of its sources (which gbox_module() already strips of main.*) come
# along so an app may keep private helpers beside main.
#
#   DEPENDS    <module>..  gbox modules, in-tree or imported, by short name
#   INCLUDES   <dir>..     include directories in addition to the module's own inc/
#   NO_MODULES             opt out of C++20 module scanning for this executable
#
# TODO: modules declared (gbox_import/gbox_library) in the same CMakeLists.txt as this call
# should be linked implicitly by scope rather than repeated in DEPENDS.
function(gbox_executable APP_NAME)
    cmake_parse_arguments(GBAPP "NO_MODULES" "" "DEPENDS;INCLUDES" ${ARGN})

    string(TOUPPER "${APP_NAME}" APP_NAME_UPPER)
    set(MODULE "${APP_NAME_UPPER}")
    set(TARGET "${${MODULE}_NAME}")

    add_executable("${TARGET}" "${${MODULE}_SRC_PATH}/main.cpp" ${${MODULE}_SRC})

    if(GBAPP_NO_MODULES)
        set_target_properties("${TARGET}" PROPERTIES CXX_SCAN_FOR_MODULES OFF)
    endif()

    target_include_directories("${TARGET}" PRIVATE ${GBAPP_INCLUDES})
    gbox_link_dependencies("${TARGET}" PRIVATE ${GBAPP_DEPENDS})

    set_property(GLOBAL APPEND PROPERTY GBOX_APPS "${TARGET}")
endfunction()
