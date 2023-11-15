#include <stdio.h>
#include <stdlib.h>

#include "assembly.h"

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    FILE *code = fopen(argv[1], "r");
    if (!code) {
        fprintf(stderr, "%s not found\n", argv[1]);
        return EXIT_FAILURE;
    }

    FILE *rom = fopen(argv[2], "wb");

    Line* line = get_lines(code);
    preprocessor(line);

    while (line) {
        if (line->value) {
            int token_count;
            char** tokens = get_tokens(line->value, &token_count);
            uint16_t opcode = get_binary(tokens, token_count);
            opcode = (opcode >> 8) | (opcode << 8);
            fwrite(&opcode, 1, sizeof(opcode), rom);
        }
        line = line->next;
    }

    fclose(rom);
    fclose(code);
    return EXIT_SUCCESS;
}
