#ifndef EMULATOR_DISASSEMBLER_H
#define EMULATOR_DISASSEMBLER_H

#include <stdlib.h>

uint8_t* get_rom(FILE *rom_file, long *rom_size);
char* get_asm_code(u_int16_t _opcode);

#endif //EMULATOR_DISASSEMBLER_H
