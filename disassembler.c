#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "assembly.h"

int disassemble(char *file, char *output) {
    FILE *rom = fopen(file, "rb");
    if (!rom) {
        fprintf(stderr, "%s not found\n", file);
        return 1;
    }

    FILE *assembly = fopen(output, "w");
    if (!assembly) {
        fprintf(stderr, "failed to open %s", output);
        return 1;
    }
    long rom_size;
    uint8_t *binary = get_rom(rom, &rom_size);
    for (int i = 0; i < rom_size + 1; i += 2) {
        uint16_t opcode = (binary[i] << 8) | binary[i + 1];
        char *instruction = get_asm_code(opcode);
        if (strlen(instruction) > 0) {
            fprintf(assembly, "%s\n", instruction);
        } else {
            fprintf(assembly, "0x%04X\n", opcode);
        }
    }
    free(binary);
    fclose(assembly);
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    disassemble(argv[1], argv[2]);
}
