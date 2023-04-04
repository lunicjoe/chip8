#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "assembly.h"

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    FILE *code = fopen(argv[1], "r");
    if (!code) {
        fprintf(stderr, "%s not found\n", argv[1]);
        return 1;
    }

    FILE *rom = fopen(argv[2], "wb");
    char *line = NULL;
    size_t length;
    while (getline(&line, &length, code) != -1) {
        line[strlen(line) - 1] = '\0';
        uint16_t opcode = get_binary(line);
        opcode = (opcode >> 8) | (opcode << 8);
        fwrite(&opcode, 1, sizeof(opcode), rom);
    }
    fclose(rom);
    fclose(code);
}
