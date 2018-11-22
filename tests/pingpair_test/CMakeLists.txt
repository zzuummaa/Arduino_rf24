cmake_minimum_required(VERSION 3.8)

project(pingpair_test)

add_arduino_executable(${PROJECT_NAME} ${board_id} printf.h)
target_sketches(${PROJECT_NAME} ${board_id} pingpair_test.pde)
link_rf24_libraries(${PROJECT_NAME})

upload_arduino_target(${PROJECT_NAME} "${board_id}" ${SERIAL_PORT})