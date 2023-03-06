#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H

#include <stdbool.h>
#include <SDL2/SDL.h>

#define PIXEL_SIZE 5
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

typedef struct {
    char graphics_memory[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint16_t opcode;
    uint8_t memory[4096];
    uint16_t pc;
    uint16_t index;
    uint8_t V[16];
    uint8_t sp;
    uint16_t stack[16];
    uint8_t flag;
    bool pressed_key[16];
} Chip8;
extern Chip8 chip8;
extern SDL_Rect rectangle;
void chip8_init();
void chip8_load_rom(char *file);
void chip8_cycle();
void chip8_clear();
void chip8_jump();
void chip8_draw();
void chip8_move();
void chip8_add();
void chip8_load_index();
void chip8_reg_dump();
void chip8_reg_load();
void chip8_get_key();
void chip8_v_pressed();
void chip8_pressed();
void chip8_call();
void chip8_return();
void chip8_font_character();
void chip8_render(SDL_Renderer *renderer);
void chip8_log();

#endif //CHIP8_CHIP8_H
