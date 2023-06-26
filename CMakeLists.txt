cmake_minimum_required(VERSION 3.0)
project(PRESSTEST)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 11)
# 将不一致代码的诊断从错误降级为警告
set(CMAKE_CXX_FLAGS -fpermissive)
# set out path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# set .a path
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# set .cpp path
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/method/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        )
# print SRC_LIST
message("SRC_LIST = ${SRC_LIST}")
# set .h path
include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(gflags)        

add_executable(exepress.out ${SRC_LIST})
target_link_libraries(exepress.out gflags pthread curl)