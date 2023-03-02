cmake_minimum_required(VERSION 3.23)
project(chip8 C)

set(CMAKE_C_STANDARD 23)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(chip8 main.c chip8.h chip8.c)

target_link_libraries(chip8 ${SDL2_LIBRARIES})