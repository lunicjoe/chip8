cmake_minimum_required(VERSION 3.23)
project(emulator C)

set(CMAKE_C_STANDARD 23)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(emulator emulator.c emulator/chip8.h emulator/chip8.c log.h log.c)
add_executable(log_reader log_reader.c)
add_executable(disassembler disassembler.c)

target_link_libraries(emulator ${SDL2_LIBRARIES})