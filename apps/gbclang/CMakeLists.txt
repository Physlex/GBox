set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

add_executable(${GBCLANG_NAME} ${GBCLANG_SRC_PATH}/main.cpp ${GBCLANG_SRC})
target_link_libraries(${GBCLANG_NAME} PRIVATE gbox::proc)
target_include_directories(${GBCLANG_NAME} PUBLIC ${GBCLANG_INC_PATH})
