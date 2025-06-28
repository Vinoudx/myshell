cmake_minimum_required(VERSION 2.8)
project(myshell)

set(CMAKE_BUILD_TYPE Debug)

aux_source_directory(. SRC)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
include_directories(include)


add_executable(myshell ${SRC})