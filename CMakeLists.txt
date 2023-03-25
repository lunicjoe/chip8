cmake_minimum_required(VERSION 3.23)
project(emulator C)

set(CMAKE_C_STANDARD 23)

find_package(SDL2 REQUIRED)
find_package(Curses REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${CURSES_INCLUDE_DIRS})

add_executable(emulator emulator.c emulator/chip8.h emulator/chip8.c log.h log.c assembly.h assembly.c)
add_executable(disassembler disassembler.c assembly.h assembly.c)

add_compile_definitions(NCURSES_LOGGING)

target_link_libraries(emulator ${SDL2_LIBRARIES})
target_link_libraries(emulator ${CURSES_LIBRARIES})
