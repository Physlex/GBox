find_package(gbox REQUIRED)
add_executable(${TREPIDATION_NAME} ${TREPIDATION_SRC_PATH}/main.cppm ${TREPIDATION_SRC})
target_link_libraries(${TREPIDATION_NAME} PRIVATE gbox::runtime gbox::core)
