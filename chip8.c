#include "chip8.h"

#include <stdbool.h>
#include <SDL2/SDL.h>

#define bytecode_log(text, ...) snprintf(instruction, 100, text, ##__VA_ARGS__)

SDL_Rect rectangle;
Chip8 chip8;

int keys[] = {
        30, 31, 32, 33,
        20, 26, 8, 21,
        4, 22, 7, 9,
        29, 27, 6, 25
};

uint8_t characters[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_init() {
    SDL_RectEmpty(&rectangle);
    rectangle.h = PIXEL_SIZE;
    rectangle.w = PIXEL_SIZE;
    chip8_clear();
    chip8.opcode = 0;
    chip8.pc = 0x200;
    chip8.sp = 0;
    for (int i = 0; i < 16; i++) {
        chip8.pressed_key[i] = false;
    }
    for (int i = 0; i < 5 * 0xf; i++) {
        chip8.memory[0x50 + i] = characters[i];
    }
    chip8.memory[0x1ff] = 3;
}

void chip8_load_rom(char *file) {
    FILE *rom = fopen(file, "rb");
    fseek(rom, 0, SEEK_END);
    const long rom_size = ftell(rom);
    rewind(rom);
    uint8_t *rom_buffer = (uint8_t*) malloc(sizeof(uint8_t) * rom_size);
    fread(rom_buffer, sizeof(uint8_t), rom_size, rom);
    fclose(rom);
    for (int i = 0; i < rom_size; i += 2) {
        chip8.memory[0x200 + i] = rom_buffer[i];
        chip8.memory[0x200 + i + 1] = rom_buffer[i + 1];
    }
    chip8.memory[0x200 + 0x1ff] = 4;
}

uint8_t get_0x00() {
    return (chip8.opcode & 0x0f00) >> 8;
}

uint8_t get_00x0() {
    return (chip8.opcode & 0x00f0) >> 4;
}

uint8_t get_00xx() {
    return chip8.opcode & 0x00ff;
}

uint16_t get_0xxx() {
    return chip8.opcode & 0x0fff;
}

void chip8_cycle() {
    for (int i = 0; i < 16; i++) {
        chip8.pressed_key[i] = SDL_GetKeyboardState(NULL)[keys[i]] == 1;
    }
    char instruction[0xff] = "";
    uint16_t instruction_address = chip8.pc;
    chip8.opcode = (chip8.memory[chip8.pc] << 8) | chip8.memory[chip8.pc + 1];
    switch (chip8.opcode & 0xf000) {
        case 0x0000:
            switch (get_00xx()) {
                case 0x00e0:
                    strncpy(instruction, "cls", 100);
                    chip8_clear();
                    break;
                case 0x00ee:
                    strncpy(instruction, "return", 100);
                    chip8_return();
                    break;
            }
            break;
        case 0x1000:
            bytecode_log("jmp 0x%X", chip8.opcode & 0xfff);
            chip8_jump();
            break;
        case 0x2000:
            bytecode_log("call 0x%X", get_0xxx());
            chip8_call();
            break;
        case 0x3000:
            bytecode_log("skip if V%X == 0x%X", get_0x00(), get_00xx());
            if (chip8.V[get_0x00()] == (get_00xx())) {
                chip8.pc += 2;
            }
        case 0x4000:
            bytecode_log("skip if V%X != 0x%X", get_0x00(), get_00xx());
            if (chip8.V[get_0x00()] != (get_00xx())) {
                chip8.pc += 2;
            }
            break;
        case 0x5000:
            bytecode_log("skip if V%X == V%X", get_0x00(), get_00x0());
            if (chip8.V[get_0x00()] == chip8.V[get_00x0()]) {
                chip8.pc += 2;
            }
        case 0x6000:
            bytecode_log("mov V%X, 0x%X", get_0x00(), get_00xx());
            chip8_move();
            break;
        case 0x7000:
            bytecode_log("add V%X, 0x%X", get_0x00(), get_00xx());
            chip8_add();
            break;
        case 0x8000:
            switch (chip8.opcode & 0x000f) {
                case 0x0:
                    bytecode_log("V%X = V%X", get_0x00(), get_00x0());
                    chip8.V[get_0x00()] = chip8.V[get_00x0()];
                    break;
                case 0x1:
                    bytecode_log("V%X |= V%X", get_0x00(), get_00x0());
                    chip8.V[get_0x00()] |= chip8.V[get_00x0()];
                    break;
                case 0x2:
                    bytecode_log("V%X &= V%X", get_0x00(), get_00x0());
                    chip8.V[get_0x00()] &= chip8.V[get_00x0()];
                    break;
                case 0x3:
                    bytecode_log("V%X ^= V%X", get_0x00(), get_00x0());
                    chip8.V[get_0x00()] ^= chip8.V[get_00x0()];
                    break;
                case 0x4:
                    bytecode_log("V%X += V%X", get_0x00(), get_00x0());
                    chip8.V[get_0x00()] += chip8.V[get_00x0()];
                    break;
                case 0x5:
                    bytecode_log("V%X -= V%X", get_0x00(), get_00x0());
                    chip8.V[get_0x00()] -= chip8.V[get_00x0()];
                    break;
                case 0x6:
                    bytecode_log("V%X >>= 1", get_0x00());
                    chip8.V[get_0x00()] >>= 1;
                    break;
                case 0x7:
                    bytecode_log("V%X = V%X - V%X", get_0x00(), get_00x0(), get_0x00());
                    chip8.V[get_0x00()] = chip8.V[get_00x0()] - chip8.V[get_0x00()];
                    break;
                case 0xe:
                    bytecode_log("V%X <<= 1", get_0x00());
                    chip8.V[get_0x00()] <<= 1;
                    break;
            }
            break;
        case 0xa000:
            bytecode_log("mvi 0x%X", get_0xxx());
            chip8_load_index();
            break;
        case 0xc000:
            break;
        case 0xd000:
            bytecode_log("draw V%X, V%X", get_0x00(), get_00x0());
            chip8_draw();
            break;
        case 0xe000:
            switch (get_00xx()) {
                case 0x9e:
                    bytecode_log("skip if V%X key (0x%X) is pressed", get_0x00(), (chip8.V[get_0x00()]));
                    chip8_v_pressed();
                    break;
                case 0xa1:
                    bytecode_log("skip if 0x%X not pressed", get_0x00());
                    chip8_pressed();
                    break;
            }
        case 0xf000:
            switch (get_00xx()) {
                case 0x1e:
                    bytecode_log("index += V%X", get_0x00());
                    chip8_add_v_to_index();
                    break;
                case 0x29:
                    bytecode_log("font character");
                    chip8_font_character();
                    break;
                case 0x55:
                    strncpy(instruction, "reg dump", 100);
                    chip8_reg_dump();
                    break;
                case 0x65:
                    strncpy(instruction, "reg load", 100);
                    chip8_reg_load();
                    break;
                case 0x0a:
                    bytecode_log("get key V%X", get_0x00());
                    chip8_get_key();
                    break;
            }
    }
    printf("0x%04x: 0x%04x %s\n", instruction_address, chip8.opcode, instruction);
    chip8.pc += 2;
}

void chip8_clear() {
    for (int pixel = 0; pixel < SCREEN_WIDTH * SCREEN_HEIGHT; pixel++) {
        chip8.graphics_memory[pixel] = 0;
    }
}

void chip8_jump() {
    chip8.pc = (chip8.opcode & 0xfff) - 2;
}

void chip8_move() {
    chip8.V[get_0x00()] = get_00xx();
}

void chip8_add() {
    chip8.V[get_0x00()] += get_00xx();
}

void chip8_load_index() {
    chip8.index = get_0xxx();
}

void chip8_reg_dump() {
    for (int i = 0; i <= get_0x00(); i++) {
        chip8.memory[0x200 + chip8.index + i] = chip8.V[i];
    }
}

void chip8_reg_load() {
    for (int i = 0; i <= get_0x00(); i++) {
        chip8.V[i] = chip8.memory[0x200 + chip8.index + i];
    }
}

void chip8_get_key() {
    int pressed = -1;
    for (int key = 0; key < 16; key++) {
        if (chip8.pressed_key[key]) {
            pressed = key;
            break;
        }
    }
    if (pressed >= 0) {
        chip8.V[get_0x00()] = pressed;
    } else {
        chip8.pc -= 2;
    }
}

void chip8_v_pressed() {
    if (chip8.pressed_key[chip8.V[get_0x00()]]) {
        chip8.pc += 2;
    }
}

void chip8_pressed() {
    if (!chip8.pressed_key[chip8.V[get_0x00()]]) {
        chip8.pc += 2;
    }
}

void chip8_call() {
    if (chip8.sp < 16) {
        chip8.stack[chip8.sp] = chip8.pc + 2;
        chip8.sp++;
        chip8.pc = get_0xxx();
        chip8.pc -= 2;
    } else {
        return;
    }
}

void chip8_return() {
    chip8.sp--;
    chip8.pc = chip8.stack[chip8.sp];
    chip8.pc -= 2;
    chip8.stack[chip8.sp] = 0;
}

void chip8_font_character() {
    chip8.index = chip8.V[get_0x00()] * 5 + 0x50;
}

void chip8_add_v_to_index() {
    chip8.index += chip8.V[get_0x00()];
}

void chip8_draw() {
    chip8.flag = 0;
    const uint8_t x_sprite = chip8.V[get_0x00()];
    const uint8_t y_sprite = chip8.V[get_00x0()];
    const uint8_t rows = chip8.opcode & 0x000f;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < 8; x++) {
            if (x_sprite + x < 64 && y_sprite + y < 32) {
                if (((chip8.memory[chip8.index + y] >> x) & 1) == 1) {
                    if (chip8.graphics_memory[(x_sprite + 8 - x) + SCREEN_WIDTH * (y_sprite + y)] == 0) {
                        chip8.graphics_memory[(x_sprite + 8 - x) + SCREEN_WIDTH * (y_sprite + y)] = 1;
                    } else {
                        chip8.graphics_memory[(x_sprite + 8 - x) + SCREEN_WIDTH * (y_sprite + y)] = 0;
                        chip8.flag = 1;
                    }
                }
            }
        }
    }
}

void chip8_render(SDL_Renderer *renderer) {
    for (int pixel = 0; pixel < SCREEN_WIDTH * SCREEN_HEIGHT; pixel++) {
        rectangle.x = pixel % SCREEN_WIDTH * PIXEL_SIZE;
        rectangle.y = (pixel - (pixel % SCREEN_WIDTH)) / SCREEN_WIDTH * PIXEL_SIZE;
        if (chip8.graphics_memory[pixel] == 1) {
            SDL_RenderFillRect(renderer, &rectangle);
        }
    }
}

void chip8_log() {
    printf("PC:0x%02X\t", chip8.pc);
    printf("I:0x%02X\t", chip8.index);
    for (int v = 0; v < 16; v++) {
        printf("V%X:0x%02X\t", v, chip8.V[v]);
    }
    printf("\n");
    printf("SP:0x%02X\t", chip8.sp);
    for (int i = 0; i < chip8.sp; i++) {
        printf("stack[0x%X]:0x%02X\t", i, chip8.stack[i]);
    }
    printf("\n");
}