cmake_minimum_required(VERSION 3.17)
project(P3 C)

set(CMAKE_C_STANDARD 99)

add_executable(P3 src/shell.c src/list.c include/list.h include/background_list.h src/background_list.c)