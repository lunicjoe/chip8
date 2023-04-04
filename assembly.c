#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "chip8.h"

uint8_t get_0x00(uint16_t opcode) {
    return (opcode & 0x0f00) >> 8;
}
uint8_t get_00x0(uint16_t opcode) {
    return (opcode & 0x00f0) >> 4;
}
uint8_t get_00xx(uint16_t opcode) {
    return opcode & 0x00ff;
}
uint16_t get_0xxx(uint16_t opcode) {
    return opcode & 0x0fff;
}

uint8_t* get_rom(FILE *rom_file, long *rom_size) {
    fseek(rom_file, 0, SEEK_END);
    *rom_size = ftell(rom_file);
    rewind(rom_file);
    uint8_t *rom = malloc(*rom_size);
    fread(rom, sizeof(uint8_t), *rom_size, rom_file);
    fclose(rom_file);
    return rom;
}

#define set_instruction(text, ...) { \
    size_t size = snprintf(NULL, 0, text, ##__VA_ARGS__) + 1; \
    instruction = realloc(instruction, size); \
    snprintf(instruction, size, text, ##__VA_ARGS__); \
}

char* get_asm_code(u_int16_t opcode) {
    char *instruction = malloc(1);
    instruction[0] = '\0';
    switch (opcode & 0xf000) {
        case 0x0000:
            switch (get_00xx(opcode)) {
                case 0x00e0:
                    set_instruction("cls");
                    break;
                case 0x00ee:
                    set_instruction("ret");
                    break;
            }
            break;
        case 0x1000:
            set_instruction("jmp 0x%X", get_0xxx(opcode));
            break;
        case 0x2000:
            set_instruction("call 0x%X", get_0xxx(opcode));
            break;
        case 0x3000:
            set_instruction("se V%X, 0x%X", get_0x00(opcode), get_00xx(opcode));
            break;
        case 0x4000:
            set_instruction("sne V%X, 0x%X", get_0x00(opcode), get_00xx(opcode));
            break;
        case 0x5000:
            set_instruction("se V%X, V%X", get_0x00(opcode), get_00x0(opcode));
            break;
        case 0x6000:
            set_instruction("ld V%X, 0x%X", get_0x00(opcode), get_00xx(opcode));
            break;
        case 0x7000:
            set_instruction("add V%X, 0x%X", get_0x00(opcode), get_00xx(opcode));
            break;
        case 0x8000:
            switch (opcode & 0x000f) {
                case 0x0:
                    set_instruction("ld V%X, V%X", get_0x00(opcode), get_00x0(opcode));
                    break;
                case 0x1:
                    set_instruction("or V%X, V%X", get_0x00(opcode), get_00x0(opcode));
                    break;
                case 0x2:
                    set_instruction("and V%X, V%X", get_0x00(opcode), get_00x0(opcode));
                    break;
                case 0x3:
                    set_instruction("xor V%X, V%X", get_0x00(opcode), get_00x0(opcode));
                    break;
                case 0x4:
                    set_instruction("add V%X, V%X", get_0x00(opcode), get_00x0(opcode));
                    break;
                case 0x5:
                    set_instruction("sub V%X, V%X", get_0x00(opcode), get_00x0(opcode));
                    break;
                case 0x6:
                    set_instruction("shr V%X", get_0x00(opcode));
                    break;
                case 0x7:
                    set_instruction("subn V%X, V%X", get_0x00(opcode), get_00x0(opcode));
                    break;
                case 0xe:
                    set_instruction("shl V%X", get_0x00(opcode));
                    break;
            }
            break;
        case 0x9000:
            set_instruction("sne V%X, V%X", get_0x00(opcode), get_00x0(opcode));
            break;
        case 0xa000:
            set_instruction("ld I, 0x%X", get_0xxx(opcode));
            break;
        case 0xb000:
            set_instruction("jmp V0, 0x%X", get_0xxx(opcode))
            break;
        case 0xc000:
            set_instruction("rnd V%X, 0x%02X", get_0x00(opcode), get_00xx(opcode));
            break;
        case 0xd000:
            set_instruction("drw V%X, V%X, 0x%02X", get_0x00(opcode), get_00x0(opcode), opcode & 0xf);
            break;
        case 0xe000:
            switch (get_00xx(opcode)) {
                case 0x9e:
                    set_instruction("skp V%X", get_0x00(opcode));
                    break;
                case 0xa1:
                    set_instruction("sknp V%X", get_0x00(opcode));
                    break;
            }
        case 0xf000:
            switch (get_00xx(opcode)) {
                case 0x7:
                    set_instruction("ld V%X, DT", get_0x00(opcode));
                    break;
                case 0x0a:
                    set_instruction("ld V%X, KEY", get_0x00(opcode));
                    break;
                case 0x15:
                    set_instruction("ld DT, V%X", get_0x00(opcode));
                    break;
                case 0x1e:
                    set_instruction("add I, V%X", get_0x00(opcode));
                    break;
                case 0x29:
                    set_instruction("font V%X", get_0x00(opcode));
                    break;
                case 0x33:
                    set_instruction("bcd V%X", get_0x00(opcode));
                    break;
                case 0x55:
                    set_instruction("ld [I], V%X", get_0x00(opcode));
                    break;
                case 0x65:
                    set_instruction("ld V%X, [I]", get_0x00(opcode));
                    break;
            }
            break;
        default:
            set_instruction("0x%04X", opcode);
    }
    return instruction;
}

#define get_value(number) strtol(tokens[number], NULL, 0)
#define is_reg(number) is_register(tokens[number])
#define get_register(number) get_register_value(tokens[number])

uint16_t set_000x(uint16_t number) {
    return number & 0xf;
}
uint16_t set_00xx(uint16_t number) {
    return number & 0xff;
}
uint16_t set_0xxx(uint16_t number) {
    return number & 0xfff;
}
uint16_t set_0x00(uint16_t number) {
    return (number << 8) & 0xf00;
}
uint16_t set_00x0(uint16_t number) {
    return (number << 4) & 0xf0;
}
bool is_register(const char *instruction) {
    return instruction[0] == 'V' || instruction[0] == 'v';
}
uint16_t get_register_value(char *instruction) {
    return strtol(&instruction[1], NULL, 16);
}

typedef enum {
    CLS, RET, JMP, CALL, SE, SNE, LD, ADD, RND, DRW, BCD, SKP, SKNP, OR, AND, XOR, SUB, SHR, SUBN, SHL, FONT, OTHER
} instruction_type;
typedef struct {
    char *string;
    instruction_type type;
} instruction;
instruction instructions[] = {
        {"cls", CLS},
        {"ret", RET},
        {"jmp", JMP},
        {"call", CALL},
        {"se", SE},
        {"sne", SNE},
        {"ld", LD},
        {"add", ADD},
        {"rnd", RND},
        {"drw", DRW},
        {"bcd", BCD},
        {"skp", SKP},
        {"sknp", SKNP},
        {"or", OR},
        {"and", AND},
        {"xor", XOR},
        {"sub", SUB},
        {"shr", SHR},
        {"subn", SUBN},
        {"shl", SHL},
        {"font", FONT},
};

const char delimiters[] = " ,";

instruction_type get_instruction(char *code) {
    char *instruction_str = strtok(code, delimiters);
    for (int i = 0; i < sizeof(instructions) / sizeof(instruction); i++) {
        if (strcmp(instruction_str, instructions[i].string) == 0) return instructions[i].type;
    }
    return OTHER;
}

uint16_t get_binary(char *code) {
    int i = 0;
    char **tokens = malloc(1 * sizeof(char*));
    tokens[i] = strtok(code, delimiters);
    do {
        i++;
        tokens = realloc(tokens, (1 + i) * sizeof(char *));
    } while ((tokens[i] = strtok(NULL, delimiters)) != NULL);
    instruction_type instruction_type = get_instruction(code);
    switch (instruction_type) {
        case CLS:
            return 0x00e0;
        case RET:
            return 0x00ee;
        case JMP:
            if (!is_reg(1)) {
                return 0x1000 + set_0xxx(get_value(1));
            } else {
                return 0xb000 + set_0xxx(get_value(2));
            }
        case CALL:
            return 0x2000 + set_0xxx(get_value(1));
        case SE:
            if (is_reg(2)) {
                return 0x5000 + set_0x00(get_register(1)) + set_00x0(get_register(2));
            } else {
                return 0x3000 + set_0x00(get_register(1)) + set_00xx(get_value(2));
            }
        case SNE:
            if (is_reg(2)) {
                return 0x9000 + set_0x00(get_register(1)) + set_00x0(get_register(2));
            } else {
                return 0x4000 + set_0x00(get_register(1)) + get_value(2);
            }
        case LD:
            if (is_reg(1)) {
                if (is_reg(2)) {
                    return 0x8000 + set_0x00(get_register(1)) + set_00x0(get_register(2));
                } else if ((strcmp(tokens[2], "[I]") == 0)) {
                    return 0xf065 + set_0x00(get_register(1));
                } else if ((strcmp(tokens[2], "DT") == 0)) {
                    return 0xf007 + set_0x00(get_register(1));
                } else if ((strcmp(tokens[2], "KEY") == 0)) {
                    return 0xf00a + set_0x00(get_register(1));
                }
                return 0x6000 + set_0x00(get_register(1)) + get_value(2);
            } else {
                if (strcmp(tokens[1], "I") == 0) {
                    return 0xa000 + set_0xxx(get_value(2));
                } else if (strcmp(tokens[1], "DT") == 0) {
                    return 0xf015 + set_0x00(get_register(2));
                } else if (strcmp(tokens[1], "[I]") == 0) {
                    return 0xf055 + set_0x00(get_register(2));
                }
            }
        case ADD:
            if (!is_reg(2)) {
                return 0x7000 + set_0x00(get_register(1)) + set_00xx(get_value(2));
            } else if (is_reg(1) && is_reg(2)) {
                return 0x8004 + set_0x00(get_register(1)) + set_00x0(get_register(2));
            } else if (tokens[1][0] == 'I') {
                return 0xf01e + set_0x00(get_register(2));
            }
        case RND:
            return 0xc000 + set_0x00(get_register(1)) + set_00xx(get_value(2));
        case DRW:
            return 0xd000 + set_0x00(get_register(1)) + set_00x0(get_register(2)) + set_000x(get_value(3));
        case BCD:
            return 0xf033 + set_0x00(get_register(1));
        case SKP:
            return 0xe09e + set_0x00(get_register(1));
        case SKNP:
            return 0xe0a1 + set_0x00(get_register(1));
        case OR:
            return 0x8001 + set_0x00(get_register(1)) + set_00x0(get_register(2));
        case AND:
            return 0x8002 + set_0x00(get_register(1)) + set_00x0(get_register(2));
        case XOR:
            return 0x8003 + set_0x00(get_register(1)) + set_00x0(get_register(2));
        case SUB:
            return 0x8005 + set_0x00(get_register(1)) + set_00x0(get_register(2));
        case SHR:
            return 0x8006 + set_0x00(get_register(1));
        case SUBN:
            return 0x8007 + set_0x00(get_register(1)) + set_00x0(get_register(2));
        case SHL:
            return 0x800e + set_0x00(get_register(1));
        case FONT:
            return 0xf029 + set_0x00(get_register(1));
        default:
            return strtol(tokens[0], NULL, 0);
    }
}
