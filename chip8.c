#include "chip8.h"

#include <stdbool.h>
#include <SDL.h>
#include <stdlib.h>
#include <time.h>
#include "log.h"
#include "assembly.h"

SDL_Rect pixel_rect;
Chip8 chip8;

int i_cycle = 0;
Chip8 *chip8_states;

char instruction[INSTRUCTION_SIZE] = "";

int keys[] = {
        SDL_SCANCODE_X, // 0
        SDL_SCANCODE_1, // 1
        SDL_SCANCODE_2, // 2
        SDL_SCANCODE_3, // 3
        SDL_SCANCODE_Q, // 4
        SDL_SCANCODE_W, // 5
        SDL_SCANCODE_E, // 6
        SDL_SCANCODE_A, // 7
        SDL_SCANCODE_S, // 8
        SDL_SCANCODE_D, // 9
        SDL_SCANCODE_Z, // A
        SDL_SCANCODE_C, // B
        SDL_SCANCODE_4, // C
        SDL_SCANCODE_R, // D
        SDL_SCANCODE_F, // E
        SDL_SCANCODE_V  // F
};

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

#define get_0x00 get_0x00(chip8.opcode)
#define get_00x0 get_00x0(chip8.opcode)
#define get_00xx get_00xx(chip8.opcode)
#define get_0xxx get_0xxx(chip8.opcode)

void chip8_init() {
    chip8_logging_init();
    srand(time(NULL));
    init_pixel();
    chip8_states = malloc(sizeof(Chip8));
    chip8.opcode = 0;
    chip8.pc = START_ADDRESS;
    chip8.sp = 0;
    for (int key = 0; key < 16; key++) {
        chip8.pressed_key[key] = false;
    }
    for (int i_byte = 0; i_byte < 5 * 0x10; i_byte++) {
        chip8.memory[FONT_ADDRESS + i_byte] = font[i_byte];
    }
}

void init_pixel() {
    SDL_RectEmpty(&pixel_rect);
    pixel_rect.h = PIXEL_SIZE;
    pixel_rect.w = PIXEL_SIZE;
}

int chip8_load_rom(char *file) {
    FILE *rom_file = fopen(file, "rb");
    if (rom_file == NULL) return 0;

    long rom_size;
    uint8_t *rom = get_rom(rom_file, &rom_size);
    for (int i_byte = 0; i_byte + 1 < rom_size; i_byte += 2) {
        chip8.memory[START_ADDRESS + i_byte] = rom[i_byte];
        chip8.memory[START_ADDRESS + i_byte + 1] = rom[i_byte + 1];
    }
    free(rom);
    return 1;
}

void (*cpu_instructions[16])();

void chip8_update(const uint8_t *keyboard, const uint8_t *previous_keyboard, bool debug) {
    if (debug) {
        if (keyboard[SDL_SCANCODE_SPACE]) {
            if (keyboard[SDL_SCANCODE_RIGHT]) chip8_forward();
            if (keyboard[SDL_SCANCODE_LEFT]) chip8_backward();
        } else {
            if (!keyboard[SDL_SCANCODE_RIGHT] && previous_keyboard[SDL_SCANCODE_RIGHT]) chip8_forward();
            if (!keyboard[SDL_SCANCODE_LEFT] && previous_keyboard[SDL_SCANCODE_LEFT]) chip8_backward();
        }
    } else {
        chip8_forward();
    }
    chip8_logging(&chip8);
}
void chip8_forward() {
    memcpy(&chip8_states[i_cycle], &chip8, sizeof(Chip8));
    i_cycle++;
    chip8_states = realloc(chip8_states, sizeof(Chip8) * (i_cycle + 1));

    if (chip8.delay_timer > 0) chip8.delay_timer--;
    for (int key = 0; key < 16; key++) {
        chip8.pressed_key[key] = SDL_GetKeyboardState(NULL)[keys[key]];
    }
    chip8.opcode = (chip8.memory[chip8.pc] << 8) | chip8.memory[chip8.pc + 1];

    cpu_instructions[(chip8.opcode & 0xf000) >> 12]();
    chip8.pc += 2;
}

void chip8_backward() {
    if (i_cycle > 0) {
        i_cycle--;
        memcpy(&chip8, &chip8_states[i_cycle], sizeof(Chip8));
        chip8_logging(&chip8);
    }
}

void chip8_render(SDL_Renderer *renderer) {
    for (int pixel = 0; pixel < SCREEN_WIDTH * SCREEN_HEIGHT; pixel++) {
        pixel_rect.x = pixel % SCREEN_WIDTH * PIXEL_SIZE;
        pixel_rect.y = (pixel - (pixel % SCREEN_WIDTH)) / SCREEN_WIDTH * PIXEL_SIZE;
        if (chip8.graphics_memory[pixel]) {
            SDL_RenderFillRect(renderer, &pixel_rect);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderDrawRect(renderer, &pixel_rect);
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0);
    }
}

void cpu_cls_ret() {
    if (get_00xx == 0xe0) {
        for (int pixel = 0; pixel < SCREEN_WIDTH * SCREEN_HEIGHT; pixel++) {
            chip8.graphics_memory[pixel] = 0;
        }
    } else if (get_00xx == 0xee) {
        chip8.sp--;
        chip8.pc = chip8.stack[chip8.sp] - 2;
        chip8.stack[chip8.sp] = 0;
    }
}
void cpu_jump() {
    chip8.pc = get_0xxx - 2;
}
void cpu_call() {
    if (chip8.sp < 16) {
        chip8.stack[chip8.sp] = chip8.pc + 2;
        chip8.sp++;
        chip8.pc = get_0xxx - 2;
    }
}
void cpu_se_Vx_byte() {
    if (chip8.V[get_0x00] == get_00xx) chip8.pc += 2;
}
void cpu_sne_Vx_byte() {
    if (chip8.V[get_0x00] != get_00xx) chip8.pc += 2;
}
void cpu_se_Vx_Vx() {
    if (chip8.V[get_0x00] == chip8.V[get_00x0]) chip8.pc += 2;
}
void cpu_load_Vx_byte() {
    chip8.V[get_0x00] = get_00xx;
}
void cpu_add_Vx_byte() {
    chip8.V[0xf] = chip8.V[get_0x00] + get_00xx > 0xff ? 1 : 0;
    chip8.V[get_0x00] += get_00xx;
}
void (*cpu_arithmetics[])();
void cpu_arithmetic() {
    if (cpu_arithmetics[chip8.opcode & 0xf] != NULL) {
        cpu_arithmetics[chip8.opcode & 0xf]();
    }
}
void cpu_sne_Vx_Vx() {
    if (chip8.V[get_0x00] != chip8.V[get_00x0]) chip8.pc += 2;
}
void cpu_load_index_byte() {
    chip8.index = get_0xxx;
}
void cpu_jump_NNN_V0() {
    chip8.pc = chip8.V[0] + get_0xxx - 2;
}
void cpu_rand() {
    chip8.V[get_0x00] = (rand() % 0xff) & get_00xx;
}
void cpu_draw() {
    const uint8_t x_sprite = chip8.V[get_0x00];
    const uint8_t y_sprite = chip8.V[get_00x0];
    const uint8_t rows = chip8.opcode & 0xf;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < 8; x++) {
            if (chip8.memory[chip8.index + y] << x & 0x80) {
                const uint16_t pixel = (x_sprite + x) % SCREEN_WIDTH + SCREEN_WIDTH * ((y_sprite + y) % SCREEN_HEIGHT);
                chip8.V[0xf] = (chip8.graphics_memory[pixel] == 1) ? 1 : 0;
                chip8.graphics_memory[pixel] ^= 1;
            }
        }
    }
}
void cpu_skp_sknp() {
    if ((get_00xx == 0x9e) && chip8.pressed_key[chip8.V[get_0x00]]
    || (get_00xx == 0xa1) && !chip8.pressed_key[chip8.V[get_0x00]]) {
        chip8.pc += 2;
    }
}
void cpu_f() {
    uint16_t number;
    switch (get_00xx) {
        case 0x7:
            chip8.V[get_0x00] = chip8.delay_timer;
            break;
        case 0x0a:
            for (int key = 0; key < 16; key++) {
                if (chip8.pressed_key[key]) {
                    chip8.V[get_0x00] = key;
                    return;
                }
            }
            chip8.pc -= 2;
            break;
        case 0x15:
            chip8.delay_timer = chip8.V[get_0x00];
            break;
        case 0x1e:
            chip8.index += chip8.V[get_0x00];
            break;
        case 0x29:
            chip8.index = FONT_ADDRESS + chip8.V[get_0x00] * 5;
            break;
        case 0x33:
            number = chip8.V[get_0x00];
            for (int i = 0; i < 3; i++) {
                chip8.memory[chip8.index + 2 - i] = number % 10;
                number /= 10;
            }
            break;
        case 0x55:
            for (int i = 0; i <= get_0x00; i++) {
                chip8.memory[chip8.index] = chip8.V[i];
                chip8.index++;
            }
            break;
        case 0x65:
            for (int i = 0; i <= get_0x00; i++) {
                chip8.V[i] = chip8.memory[chip8.index];
                chip8.index++;
            }
            break;
    }
}
// cpu arithmetics
void cpu_load_Vx_Vx() {
    chip8.V[get_0x00] = chip8.V[get_00x0];
}
void cpu_or() {
    chip8.V[get_0x00] |= chip8.V[get_00x0];
    chip8.V[0xf] = 0;
}
void cpu_and() {
    chip8.V[get_0x00] &= chip8.V[get_00x0];
    chip8.V[0xf] = 0;
}
void cpu_xor() {
    chip8.V[get_0x00] ^= chip8.V[get_00x0];
    chip8.V[0xf] = 0;
}
void cpu_add() {
    uint8_t flag = (chip8.V[get_0x00] + chip8.V[get_00x0]) > 0xff ? 1: 0;
    chip8.V[get_0x00] += chip8.V[get_00x0];
    chip8.V[0xf] = flag;
}
void cpu_sub() {
    uint8_t flag = (chip8.V[get_0x00] - chip8.V[get_00x0]) < 0 ? 0 : 1;
    chip8.V[get_0x00] -= chip8.V[get_00x0];
    chip8.V[0xf] = flag;
}
void cpu_shr() {
    chip8.V[get_0x00] = chip8.V[get_00x0];
    uint8_t flag = chip8.V[get_0x00] & 1;
    chip8.V[get_0x00] >>= 1;
    chip8.V[0xf] = flag;
}
void cpu_subn() {
    uint8_t flag = chip8.V[get_00x0] - chip8.V[get_0x00] < 0 ? 0 : 1;
    chip8.V[get_0x00] = chip8.V[get_00x0] - chip8.V[get_0x00];
    chip8.V[0xf] = flag;
}
void cpu_shl() {
    chip8.V[get_0x00] = chip8.V[get_00x0];
    uint8_t flag = (chip8.V[get_0x00] & 0x80) >> 7;
    chip8.V[get_0x00] <<= 1;
    chip8.V[0xf] = flag;
}

void (*cpu_instructions[16])() = {
        &cpu_cls_ret, &cpu_jump, &cpu_call, &cpu_se_Vx_byte, &cpu_sne_Vx_byte, &cpu_se_Vx_Vx, &cpu_load_Vx_byte,
        &cpu_add_Vx_byte, &cpu_arithmetic, &cpu_sne_Vx_Vx, &cpu_load_index_byte, &cpu_jump_NNN_V0,
        &cpu_rand, &cpu_draw, &cpu_skp_sknp, &cpu_f
};
void (*cpu_arithmetics[])() = {
        &cpu_load_Vx_Vx, &cpu_or, &cpu_and, &cpu_xor, &cpu_add, &cpu_sub, &cpu_shr, &cpu_subn,
        NULL, NULL, NULL, NULL, NULL, NULL, &cpu_shl,
};
