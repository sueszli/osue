cmake_minimum_required(VERSION 3.18)
project(mygrep C)

set(CMAKE_C_STANDARD 99)

add_executable(mygrep main.c grep.c grep.h)
target_compile_options(mygrep PRIVATE -pedantic -Wall)
