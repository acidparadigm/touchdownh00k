cmake_minimum_required(VERSION 2.8.4)
project(t0uchdownh00k)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -std=c++11")
set(EXECUTABLE_OUTPUT_PATH ./build)

set(SOURCE_FILES log.cpp remote.cpp netvar.cpp hack.cpp main.cpp)
add_executable(t0uchdownh00k ${SOURCE_FILES})
target_link_libraries(t0uchdownh00k "-lX11")
