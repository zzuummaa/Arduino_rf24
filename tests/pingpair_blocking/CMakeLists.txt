cmake_minimum_required(VERSION 3.8)

project(pingpair_blocking)

add_arduino_executable(${PROJECT_NAME} ${board_id} printf.h)
target_sketches(${PROJECT_NAME} ${board_id} pingpair_blocking.pde)
link_rf24_libraries(${PROJECT_NAME} ${board_id})

upload_arduino_target(${PROJECT_NAME} "${board_id}" ${SERIAL_PORT})