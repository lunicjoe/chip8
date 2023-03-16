#include "chip8.h"

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include "../log.h"

SDL_Rect rectangle;
Chip8 chip8;

int cycle;
Chip8 *cycles;

char instruction[INSTRUCTION_SIZE] = "";

int keys[] = {27, 30, 31, 32, 20, 26, 8, 4, 22, 7, 29, 6, 33, 21, 9, 25};

uint8_t font[] = {
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

void chip8_init() {
    srand(time(NULL));
    cycles = malloc(sizeof(Chip8));
    SDL_RectEmpty(&rectangle);
    rectangle.h = PIXEL_SIZE;
    rectangle.w = PIXEL_SIZE;
    chip8_clear();
    chip8.opcode = 0;
    chip8.pc = START_ADDRESS;
    chip8.sp = 0;
    for (int i = 0; i < 16; i++) {
        chip8.pressed_key[i] = false;
    }
    for (int i = 0; i < 5 * 0x10; i++) {
        chip8.memory[FONT_ADDRESS + i] = font[i];
    }
}

int chip8_load_rom(char *file) {
    FILE *rom = fopen(file, "rb");
    if (rom == NULL) return 0;
    fseek(rom, 0, SEEK_END);
    const long rom_size = ftell(rom);
    rewind(rom);
    uint8_t *rom_buffer = (uint8_t*) malloc(sizeof(uint8_t) * rom_size);
    fread(rom_buffer, sizeof(uint8_t), rom_size, rom);
    fclose(rom);
    for (int i = 0; i < rom_size; i += 2) {
        chip8.memory[START_ADDRESS + i] = rom_buffer[i];
        chip8.memory[START_ADDRESS + i + 1] = rom_buffer[i + 1];
    }
    return 1;
}
void (*chip_instructions[16])();
void chip8_cycle() {
    memcpy(&cycles[cycle], &chip8, sizeof(Chip8));
    cycle++;
    cycles = realloc(cycles, sizeof(Chip8) * (cycle + 1));

    if (chip8.delay_timer > 0) chip8.delay_timer--;
    for (int i = 0; i < 16; i++) {
        chip8.pressed_key[i] = SDL_GetKeyboardState(NULL)[keys[i]];
    }
    uint16_t instruction_address = chip8.pc;
    chip8.opcode = (chip8.memory[chip8.pc] << 8) | chip8.memory[chip8.pc + 1];

    chip_instructions[(chip8.opcode & 0xf000) >> 12]();
    chip8_log(&chip8, instruction_address, instruction);
    chip8.pc += 2;
}

void chip8_backward() {
    if (cycle > 0) {
        cycle--;
        memcpy(&chip8, &cycles[cycle], sizeof(Chip8));
    }
}

void chip8_render(SDL_Renderer *renderer) {
    for (int pixel = 0; pixel < SCREEN_WIDTH * SCREEN_HEIGHT; pixel++) {
        rectangle.x = pixel % SCREEN_WIDTH * PIXEL_SIZE;
        rectangle.y = (pixel - (pixel % SCREEN_WIDTH)) / SCREEN_WIDTH * PIXEL_SIZE;
        if (chip8.graphics_memory[pixel]) SDL_RenderFillRect(renderer, &rectangle);
        SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 0);
        SDL_RenderDrawRect(renderer, &rectangle);
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0);
    }
}
void chip8_clear() {
    for (int pixel = 0; pixel < SCREEN_WIDTH * SCREEN_HEIGHT; pixel++) chip8.graphics_memory[pixel] = 0;
}
void cpu_cls_ret() {
    if (get_00xx() == 0xe0) {
        bytecode_log("cls");
        chip8_clear();
    } else if (get_00xx() == 0xee) {
        bytecode_log("ret");
        chip8.sp--;
        chip8.pc = chip8.stack[chip8.sp] - 2;
        chip8.stack[chip8.sp] = 0;
    }
}
void cpu_jump() {
    bytecode_log("jmp 0x%X", get_0xxx());
    chip8.pc = get_0xxx() - 2;
}
void cpu_call() {
    bytecode_log("call 0x%X", get_0xxx());
    if (chip8.sp < 16) {
        chip8.stack[chip8.sp] = chip8.pc + 2;
        chip8.sp++;
        chip8.pc = get_0xxx() - 2;
    }
}
void cpu_3() {
    bytecode_log("se V%X, 0x%X", get_0x00(), get_00xx());
    if (chip8.V[get_0x00()] == get_00xx()) chip8.pc += 2;
}
void cpu_4() {
    bytecode_log("sne V%X, 0x%X", get_0x00(), get_00xx());
    if (chip8.V[get_0x00()] != get_00xx()) chip8.pc += 2;
}
void cpu_5() {
    bytecode_log("se V%X, V%X", get_0x00(), get_00x0());
    if (chip8.V[get_0x00()] == chip8.V[get_00x0()]) chip8.pc += 2;
}
void cpu_6() {
    bytecode_log("ld V%X, 0x%X", get_0x00(), get_00xx());
    chip8.V[get_0x00()] = get_00xx();
}
void cpu_add() {
    bytecode_log("add V%X, 0x%X", get_0x00(), get_00xx());
    chip8.V[0xf] = chip8.V[get_0x00()] + get_00xx() > 0xff ? 1 : 0;
    chip8.V[get_0x00()] += get_00xx();
}
void (*chip_arithmetic[])();
void cpu_arithmetic() {
    if (chip_arithmetic[chip8.opcode & 0xf] != NULL) chip_arithmetic[chip8.opcode & 0xf]();
}
void cpu_9() {
    bytecode_log("sne V%X, V%X", get_0x00(), get_00x0());
    if (chip8.V[get_0x00()] != chip8.V[get_00x0()]) chip8.pc += 2;
}
void cpu_a() {
    bytecode_log("ld I, 0x%X", get_0xxx());
    chip8.index = get_0xxx();
}
void cpu_rand() {
    bytecode_log("rnd V%X, 0x%02X", get_0x00(), get_00xx());
    chip8.V[get_0x00()] = (rand() % 0xff) & get_00xx();
}
void cpu_draw() {
    bytecode_log("drw V%X, V%X, 0x%02X", get_0x00(), get_00x0(), chip8.opcode & 0xf);
    const uint8_t x_sprite = chip8.V[get_0x00()];
    const uint8_t y_sprite = chip8.V[get_00x0()];
    const uint8_t rows = chip8.opcode & 0xf;
    for (int y = 0; y < rows; y++) {
        uint8_t sprite_row = chip8.memory[chip8.index + y];
        for (int x = 0; x < 8; x++) {
            if ((x_sprite + x < SCREEN_WIDTH) && (y_sprite + y < SCREEN_HEIGHT)) {
                if ((sprite_row & (0x80 >> x)) != 0) {
                    const uint16_t pixel = (x_sprite + x) + SCREEN_WIDTH * (y_sprite + y);
                    chip8.V[0xf] = chip8.graphics_memory[pixel] ? 1 : 0;
                    chip8.graphics_memory[pixel] ^= 1;
                }
            }
        }
    }
}
void cpu_e() {
    if (get_00xx() == 0x9e) {
        bytecode_log("skp V%X", get_0x00());
        if (chip8.pressed_key[chip8.V[get_0x00()]]) chip8.pc += 2;
    } else if (get_00xx() == 0xa1) {
        bytecode_log("sknp V%X", get_0x00());
        if (!chip8.pressed_key[chip8.V[get_0x00()]]) chip8.pc += 2;
    }
}
void cpu_f() {
    switch (get_00xx()) {
        case 0x7:
            bytecode_log("ld V%X, DT", get_0x00());
            chip8.V[get_0x00()] = chip8.delay_timer;
            break;
        case 0x0a:
            bytecode_log("ld V%X, KEY", get_0x00());
            for (int key = 0; key < 16; key++) {
                if (chip8.pressed_key[key]) {
                    chip8.V[get_0x00()] = key;
                    return;
                }
            }
            chip8.pc -= 2;
            break;
        case 0x15:
            bytecode_log("ld DT, V%X", get_0x00());
            chip8.delay_timer = chip8.V[get_0x00()];
            break;
        case 0x1e:
            bytecode_log("add I, V%X", get_0x00());
            chip8.index += chip8.V[get_0x00()];
            break;
        case 0x29:
            bytecode_log("ld I, FONT(V%X)", get_0x00());
            chip8.index = FONT_ADDRESS + chip8.V[get_0x00()] * 5;
            break;
        case 0x33:
            bytecode_log("bcd V%X", get_0x00());
            int number = chip8.V[get_0x00()];
            for (int i = 0; i < 3; i++) {
                chip8.memory[chip8.index + 2 - i] = number % 10;
                number /= 10;
            }
            break;
        case 0x55:
            bytecode_log("registers dump");
            for (int i = 0; i <= get_0x00(); i++) {
                chip8.memory[START_ADDRESS + chip8.index + i] = chip8.V[i];
            }
            break;
        case 0x65:
            bytecode_log("registers load");
            for (int i = 0; i <= get_0x00(); i++) {
                chip8.V[i] = chip8.memory[chip8.index + i];
            }
            break;
    }
}

void cpu_arithmetic_0() {
    bytecode_log("ld V%X, V%X", get_0x00(), get_00x0());
    chip8.V[get_0x00()] = chip8.V[get_00x0()];
}
void cpu_arithmetic_1() {
    bytecode_log("or V%X, V%X", get_0x00(), get_00x0());
    chip8.V[get_0x00()] |= chip8.V[get_00x0()];
}
void cpu_arithmetic_2() {
    bytecode_log("and V%X, V%X", get_0x00(), get_00x0());
    chip8.V[get_0x00()] &= chip8.V[get_00x0()];

}
void cpu_arithmetic_3() {
    bytecode_log("xor V%X, V%X", get_0x00(), get_00x0());
    chip8.V[get_0x00()] ^= chip8.V[get_00x0()];
}
void cpu_arithmetic_4() {
    bytecode_log("add V%X, V%X", get_0x00(), get_00x0());
    chip8.V[0xf] = chip8.V[get_0x00()] + chip8.V[get_00x0()] > 0xff ? 1: 0;
    chip8.V[get_0x00()] += chip8.V[get_00x0()];
}
void cpu_arithmetic_5() {
    bytecode_log("sub V%X, V%X", get_0x00(), get_00x0());
    chip8.V[0xf] = chip8.V[get_0x00()] -= chip8.V[get_00x0()] < 0 ? 1 : 0;
    chip8.V[get_0x00()] -= chip8.V[get_00x0()];
}
void cpu_arithmetic_6() {
    bytecode_log("shr V%X", get_0x00());
    chip8.V[0xf] = chip8.V[get_0x00()] & 1;
    chip8.V[get_0x00()] >>= 1;
}
void cpu_arithmetic_7() {
    bytecode_log("subn V%X, V%X", get_0x00(), get_00x0());
    chip8.V[0xf] = chip8.V[get_00x0()] - chip8.V[get_0x00()] < 0 ? 0 : 1;
    chip8.V[get_0x00()] = chip8.V[get_00x0()] - chip8.V[get_0x00()];
}
void cpu_arithmetic_e() {
    bytecode_log("shl V%X <<= 1", get_0x00());
    chip8.V[0xf] = chip8.V[get_0x00()] & 0x8000;
    chip8.V[get_0x00()] <<= 1;
}

void (*chip_instructions[16])() = {
        &cpu_cls_ret, &cpu_jump, &cpu_call, &cpu_3, &cpu_4, &cpu_5, &cpu_6, &cpu_add, &cpu_arithmetic, &cpu_9, &cpu_a, NULL, &cpu_rand, &cpu_draw, &cpu_e, &cpu_f
};
void (*chip_arithmetic[])() = {
        &cpu_arithmetic_0, &cpu_arithmetic_1, &cpu_arithmetic_2, &cpu_arithmetic_3, &cpu_arithmetic_4, &cpu_arithmetic_5,
        &cpu_arithmetic_6, &cpu_arithmetic_7, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &cpu_arithmetic_e,
};
