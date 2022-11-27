cmake_minimum_required(VERSION 3.20)
project(mycompress C)

set(CMAKE_C_STANDARD 99)

add_executable(mycompress main.c mycompress.c mycompress.h input.c input.h output.c output.h)
