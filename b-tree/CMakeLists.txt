cmake_minimum_required(VERSION 3.5)
project(data_structures C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g -fno-omit-frame-pointer")

add_executable(data_structures main.c btree.c)
#target_link_libraries(data_structures profiler)