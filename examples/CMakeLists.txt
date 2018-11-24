cmake_minimum_required(VERSION 3.8)

project(Examples LANGUAGES C CXX)

file(GLOB DIRS "*")
foreach(_D ${DIRS})
    if(IS_DIRECTORY ${_D})
        get_filename_component(_d_name "${_D}" NAME)
        message(STATUS "Load example: ${_d_name}")

        file(GLOB_RECURSE CPPS "${_d_name}/*.cpp" "${_d_name}/*.c")
        file(GLOB_RECURSE HEADERS "${_d_name}/*.h" "${_d_name}/*.hpp")
        file(GLOB_RECURSE SKETCHES "${_d_name}/*.ino" "${_d_name}/*.pde")

        add_arduino_executable(${_d_name} ${board_id} ${CPPS} ${HEADERS} ${SHARED_HEADERS} ${SHARED_SOURCES})
        if (SKETCHES)
            target_sketches(${_d_name} ${board_id} ${SKETCHES})
        endif()
        link_rf24_libraries(${_d_name})
        upload_arduino_target(${_d_name} "${board_id}" COM5)
        upload_arduino_target(${_d_name} "${board_id}" COM6)
    endif()
endforeach(_D)

#add_arduino_executable(pingpair_ack ${board_id})
#target_sketches(pingpair_ack ${board_id} pingpair_ack/pingpair_ack.ino)
#link_rf24_libraries(pingpair_ack)
#
#add_arduino_executable(pingpair_dyn ${board_id})
#target_sketches(pingpair_dyn ${board_id} pingpair_dyn/pingpair_dyn.ino)
#link_rf24_libraries(pingpair_dyn)