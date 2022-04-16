cmake_minimum_required(VERSION 3.10)

project(scc)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -ffunction-sections")
set(CMAKE_C_STANDARD 11)

aux_source_directory(. SOURCE_DIRECTORY)
add_compile_options(-march=native)
add_executable(scc ${SOURCE_DIRECTORY})
