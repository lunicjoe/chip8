#ifndef EMULATOR_ASSEMBLY_H
#define EMULATOR_ASSEMBLY_H

#include <stdlib.h>

uint8_t* get_rom(FILE *rom_file, long *rom_size);
char* get_asm_code(u_int16_t opcode);
uint16_t get_binary(char *code);

#endif //EMULATOR_ASSEMBLY_H
