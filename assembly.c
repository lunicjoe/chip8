#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "emulator/chip8.h"

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

char* get_asm_code(u_int16_t _opcode) {
    char *instruction = malloc(1);
    instruction[0] = '\0';
    switch (_opcode & 0xf000) {
        case 0x0000:
            switch (get_00xx(_opcode)) {
                case 0x00e0:
                    set_instruction("cls");
                    break;
                case 0x00ee:
                    set_instruction("ret");
                    break;
            }
            break;
        case 0x1000:
            set_instruction("jmp 0x%X", get_0xxx(_opcode));
            break;
        case 0x2000:
            set_instruction("call 0x%X", get_0xxx(_opcode));
            break;
        case 0x3000:
            set_instruction("se V%X, 0x%X", get_0x00(_opcode), get_00xx(_opcode));
            break;
        case 0x4000:
            set_instruction("sne V%X, 0x%X", get_0x00(_opcode), get_00xx(_opcode));
            break;
        case 0x5000:
            set_instruction("se V%X, V%X", get_0x00(_opcode), get_00x0(_opcode));
            break;
        case 0x6000:
            set_instruction("ld V%X, 0x%X", get_0x00(_opcode), get_00xx(_opcode));
            break;
        case 0x7000:
            set_instruction("add V%X, 0x%X", get_0x00(_opcode), get_00xx(_opcode));
            break;
        case 0x8000:
            switch (_opcode & 0x000f) {
                case 0x0:
                    set_instruction("ld V%X, V%X", get_0x00(_opcode), get_00x0(_opcode));
                    break;
                case 0x1:
                    set_instruction("or V%X, V%X", get_0x00(_opcode), get_00x0(_opcode));
                    break;
                case 0x2:
                    set_instruction("and V%X, V%X", get_0x00(_opcode), get_00x0(_opcode));
                    break;
                case 0x3:
                    set_instruction("xor V%X, V%X", get_0x00(_opcode), get_00x0(_opcode));
                    break;
                case 0x4:
                    set_instruction("add V%X, V%X", get_0x00(_opcode), get_00x0(_opcode));
                    break;
                case 0x5:
                    set_instruction("sub V%X, V%X", get_0x00(_opcode), get_00x0(_opcode));
                    break;
                case 0x6:
                    set_instruction("shr V%X", get_0x00(_opcode));
                    break;
                case 0x7:
                    set_instruction("subn V%X, V%X", get_0x00(_opcode), get_00x0(_opcode));
                    break;
                case 0xe:
                    set_instruction("shl V%X", get_0x00(_opcode));
                    break;
            }
            break;
        case 0x9000:
            set_instruction("sne V%X, V%X", get_0x00(_opcode), get_00x0(_opcode));
            break;
        case 0xa000:
            set_instruction("ld I, 0x%X", get_0xxx(_opcode));
            break;
        case 0xb000:
            set_instruction("jmp V0, 0x%X", get_0xxx(_opcode))
            break;
        case 0xc000:
            set_instruction("rnd V%X, 0x%02X", get_0x00(_opcode), get_00xx(_opcode));
            break;
        case 0xd000:
            set_instruction("drw V%X, V%X, 0x%02X", get_0x00(_opcode), get_00x0(_opcode), _opcode & 0xf);
            break;
        case 0xe000:
            switch (get_00xx(_opcode)) {
                case 0x9e:
                    set_instruction("skp V%X", get_0x00(_opcode));
                    break;
                case 0xa1:
                    set_instruction("sknp V%X", get_0x00(_opcode));
                    break;
            }
        case 0xf000:
            switch (get_00xx(_opcode)) {
                case 0x7:
                    set_instruction("ld V%X, DT", get_0x00(_opcode));
                    break;
                case 0x0a:
                    set_instruction("ld V%X, KEY", get_0x00(_opcode));
                    break;
                case 0x15:
                    set_instruction("ld DT, V%X", get_0x00(_opcode));
                    break;
                case 0x1e:
                    set_instruction("add I, V%X", get_0x00(_opcode));
                    break;
                case 0x29:
                    set_instruction("ld I, FONT(V%X)", get_0x00(_opcode));
                    break;
                case 0x33:
                    set_instruction("bcd V%X", get_0x00(_opcode));
                    break;
                case 0x55:
                    set_instruction("ld [I], V%X", get_0x00(_opcode));
                    break;
                case 0x65:
                    set_instruction("ld V%X, [I]", get_0x00(_opcode));
                    break;
            }
            break;
        default:
                set_instruction("0x%04X", _opcode);
    }
    return instruction;
}
