cmake_minimum_required(VERSION 3.23)
project(chip8 C)

set(CMAKE_C_STANDARD 23)

option(NCURSES_LOGGING "Enable TUI logging." OFF)

find_package(SDL2 REQUIRED)
if (NCURSES_LOGGING)
    find_package(Curses REQUIRED)
endif()

include_directories(${SDL2_INCLUDE_DIRS})
if (NCURSES_LOGGING)
    include_directories(${CURSES_INCLUDE_DIRS})
endif()

add_executable(emulator emulator.c chip8.h chip8.c log.h log.c assembly.h assembly.c)
add_executable(disassembler disassembler.c assembly.h assembly.c)
add_executable(assembler assembler.c assembly.h assembly.c)

if (NCURSES_LOGGING)
    add_compile_definitions(NCURSES_LOGGING)
    target_link_libraries(emulator ${CURSES_LIBRARIES})
endif()
target_link_libraries(emulator ${SDL2_LIBRARIES})
