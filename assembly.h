#ifndef EMULATOR_ASSEMBLY_H
#define EMULATOR_ASSEMBLY_H

#include <stdio.h>
#include <stdint.h>

typedef struct line {
    char *value;
    struct line *next;
} Line;

typedef struct {
    char *name;
    int address;
} Label;

typedef enum {
    CLS, RET, JMP, CALL, SE, SNE, LD, ADD, RND, DRW, BCD, SKP, SKNP, OR, AND, XOR, SUB, SHR, SUBN, SHL, FONT, OTHER
} instruction_type;

typedef struct {
    char *string;
    instruction_type type;
} Instruction;

#define set_instruction(text, ...) { \
    size_t size = snprintf(NULL, 0, text, ##__VA_ARGS__) + 1; \
    instruction = realloc(instruction, size); \
    snprintf(instruction, size, text, ##__VA_ARGS__); \
}

extern int label_count;
extern Label *labels;

uint8_t *get_rom(FILE *rom_file, long *rom_size);

char *get_asm_code(uint16_t opcode);

Line *get_lines(FILE *code);

void preprocessor(Line *line);

void get_label(char **line);

char **get_tokens(char *line, int *token_count);

uint16_t get_binary(char **tokens, int token_count);

#endif //EMULATOR_ASSEMBLY_H
