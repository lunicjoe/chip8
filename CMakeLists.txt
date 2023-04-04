cmake_minimum_required(VERSION 3.23)
project(chip8 C)

set(CMAKE_C_STANDARD 23)

find_package(SDL2 REQUIRED)
find_package(Curses REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${CURSES_INCLUDE_DIRS})

add_executable(emulator emulator.c chip8.h chip8.c log.h log.c assembly.h assembly.c)
add_executable(disassembler disassembler.c assembly.h assembly.c)
add_executable(assembler assembler.c assembly.h assembly.c)

add_compile_definitions(NCURSES_LOGGING)

target_link_libraries(emulator ${SDL2_LIBRARIES} ${CURSES_LIBRARIES})
