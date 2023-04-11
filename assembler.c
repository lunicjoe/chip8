#include <stdio.h>
#include "assembly.h"

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    FILE *code = fopen(argv[1], "r");
    if (!code) {
        fprintf(stderr, "%s not found\n", argv[1]);
        return 1;
    }

    FILE *rom = fopen(argv[2], "wb");

    char **lines;
    int line_count = get_lines(code, &lines);
    preprocessor(lines, line_count);

    for (int i = 0; i < line_count; i++) {
        if (lines[i]) {
            int token_count;
            char **tokens = get_tokens(lines[i], &token_count);
            uint16_t opcode = get_binary(tokens, token_count);
            opcode = (opcode >> 8) | (opcode << 8);
            fwrite(&opcode, 1, sizeof(opcode), rom);
        }
    }

    fclose(rom);
    fclose(code);
}
