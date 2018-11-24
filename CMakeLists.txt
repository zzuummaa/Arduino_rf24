cmake_minimum_required(VERSION 3.8.2)

project(Arduino_rf24)
get_board_id(board_id nano atmega328)
set(SERIAL_PORT COM6)

macro(link_rf24_libraries target_name)
    find_arduino_library(SPI_lib_${target_name} SPI ${board_id})
    find_arduino_library(RF24_lib_${target_name} RF24 ${board_id} 3RD_PARTY)
    link_arduino_library(RF24_lib_${target_name} SPI_lib_${target_name} ${board_id})
    link_arduino_library(${target_name} RF24_lib_${target_name} ${board_id})
endmacro()

include_directories(shared)
file(GLOB_RECURSE SHARED_HEADERS "shared/*.h")
file(GLOB_RECURSE SHARED_SOURCES "shared/*.c" "shared/*.cpp")

message(STATUS "ARDUINO_SDK_LIBRARIES_PATH: ${ARDUINO_SDK_LIBRARIES_PATH}")
add_subdirectory(tests)
add_subdirectory(examples)