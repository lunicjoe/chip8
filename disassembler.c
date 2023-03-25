#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <stdint.h>
#include "assembly.h"

int disassemble(char *file) {
    FILE *rom = fopen(file, "rb");
    if (!rom) {
        fprintf(stderr, "%s not found", file);
        return 1;
    }

    char *asm_file;
    asm_file = malloc(strlen(basename(file)) + 5);
    snprintf(asm_file, strlen(basename(file)) + 5, "%s.asm", basename(file));
    FILE *assembly = fopen(asm_file, "w");
    if (!assembly) {
        fprintf(stderr, "failed to open %s", asm_file);
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
}


int main(int argc, char *argv[]) {
    if (argc < 1) return 1;
    printf("disassemble %s to %s.asm\n", argv[1], argv[1]);
    disassemble(argv[1]);
}