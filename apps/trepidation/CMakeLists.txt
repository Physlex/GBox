gbox_import(sdl3 PACKAGE SDL3 COMPONENTS SDL3-shared LINK SDL3::SDL3-shared)

add_executable(${TREPIDATION_NAME} ${TREPIDATION_SRC_PATH}/main.cpp ${TREPIDATION_SRC})
target_link_libraries(${TREPIDATION_NAME} PRIVATE gbox::runtime gbox::core import::sdl3)
