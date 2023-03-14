#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bits/stdint-uintn.h>

uint16_t opcode;
uint8_t get_0x00() {
    return (opcode & 0x0f00) >> 8;
}
uint8_t get_00x0() {
    return (opcode & 0x00f0) >> 4;
}
uint8_t get_00xx() {
    return opcode & 0x00ff;
}
uint16_t get_0xxx() {
    return opcode & 0x0fff;
}

char* get_chasm_file(char rom[]) {
    char *chasm_file = malloc(strlen(rom) + 10);
    strcpy(chasm_file, rom);
    strtok(chasm_file, ".");
    strcat(chasm_file, ".chasm");
    return chasm_file;
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

char *instruction;
#define set_instruction(text, ...) { \
    ssize_t size = snprintf(NULL, 0, text, ##__VA_ARGS__); \
    instruction = realloc(instruction, size + 1);          \
    snprintf(instruction, size + 1, text, ##__VA_ARGS__); \
}

char *get_instruction() {
    switch (opcode & 0xf000) {
        case 0x0000:
            switch (get_00xx()) {
                case 0x00e0:
                    set_instruction("cls");
                    break;
                case 0x00ee:
                    set_instruction("ret");
                    break;
            }
            break;
        case 0x1000:
            set_instruction("jmp 0x%X", get_0xxx());
            break;
        case 0x2000:
            set_instruction("call 0x%X", get_0xxx());
            break;
        case 0x3000:
            set_instruction("se V%X, 0x%X", get_0x00(), get_00xx());
            break;
        case 0x4000:
            set_instruction("sne V%X, 0x%X", get_0x00(), get_00xx());
            break;
        case 0x5000:
            set_instruction("se V%X, V%X", get_0x00(), get_00x0());
            break;
        case 0x6000:
            set_instruction("ld V%X, 0x%X", get_0x00(), get_00xx());
            break;
        case 0x7000:
            set_instruction("add V%X, 0x%X", get_0x00(), get_00xx());
            break;
        case 0x8000:
            switch (opcode & 0x000f) {
                case 0x0:
                    set_instruction("ld V%X, V%X", get_0x00(), get_00x0());
                    break;
                case 0x1:
                    set_instruction("or V%X, V%X", get_0x00(), get_00x0());
                    break;
                case 0x2:
                    set_instruction("and V%X, V%X", get_0x00(), get_00x0());
                    break;
                case 0x3:
                    set_instruction("xor V%X, V%X", get_0x00(), get_00x0());
                    break;
                case 0x4:
                    set_instruction("add V%X, V%X", get_0x00(), get_00x0());
                    break;
                case 0x5:
                    set_instruction("sub V%X, V%X", get_0x00(), get_00x0());
                    break;
                case 0x6:
                    set_instruction("shr V%X", get_0x00());
                    break;
                case 0x7:
                    set_instruction("subn V%X, V%X", get_0x00(), get_00x0());
                    break;
                case 0xe:
                    set_instruction("shl V%X", get_0x00());
                    break;
            }
            break;
        case 0x9000:
            set_instruction("sne V%X, V%X", get_0x00(), get_00x0());
            break;
        case 0xa000:
            set_instruction("ld I, 0x%X", get_0xxx());
            break;
        case 0xc000:
            set_instruction("rnd V%X, 0x%02X", get_0x00(), get_00xx());
            break;
        case 0xd000:
            set_instruction("drw V%X, V%X, 0x%02X", get_0x00(), get_00x0(), opcode & 0xf);
            break;
        case 0xe000:
            switch (get_00xx()) {
                case 0x9e:
                    set_instruction("skp V%X", get_0x00());
                    break;
                case 0xa1:
                    set_instruction("sknp V%X", get_0x00());
                    break;
            }
        case 0xf000:
            switch (get_00xx()) {
                case 0x7:
                    set_instruction("ld V%X, DT", get_0x00());
                    break;
                case 0x0a:
                    set_instruction("ld V%X, KEY", get_0x00());
                    break;
                case 0x15:
                    set_instruction("ld DT, V%X", get_0x00());
                    break;
                case 0x1e:
                    set_instruction("add I, V%X", get_0x00());
                    break;
                case 0x29:
                    set_instruction("ld I, FONT(V%X)", get_0x00());
                    break;
                case 0x33:
                    set_instruction("BCD V%X", get_0x00());
                    break;
                case 0x55:
                    set_instruction("registers dump");
                    break;
                case 0x65:
                    set_instruction("registers load");
                    break;
            }
            break;
        default:
            set_instruction("0x%04X", opcode);
    }
    if (strlen(instruction) == 0) set_instruction("0x%04X", opcode);
    return instruction;
}

int main(int argc, char *argv[]) {
    instruction = malloc(1);
    instruction[0] = '\0';
    FILE *chasm_file;
    char *chasm_path = get_chasm_file(argv[1]);
    chasm_file = fopen(chasm_path, "w");
    free(chasm_path);

    FILE *rom_file = fopen(argv[1], "rb");
    if (rom_file == NULL) {
        fprintf(stderr, "%s not found\n", argv[1]);
        return EXIT_FAILURE;
    }

    long rom_size;
    uint8_t *rom = get_rom(rom_file, &rom_size);
    for (int i = 0; i < rom_size; i += 2) {
        if (i + 1 < rom_size) {
            opcode = (rom[i] << 8) | rom[i + 1];
            instruction = get_instruction();
            fprintf(chasm_file, "%s\n", instruction);
        }
    }
    free(rom);

    return EXIT_SUCCESS;
}