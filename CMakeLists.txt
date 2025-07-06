cmake_minimum_required(VERSION 2.8)
project(myshell)

set(CMAKE_BUILD_TYPE Debug)

# add_definitions(-DDEBUG)

aux_source_directory(. SRC)
list(REMOVE_ITEM SRC
    "./syntax_test.c"
    "./autocomplete_test.c"
)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
include_directories(include)


add_executable(myshell ${SRC} ${SRC1})