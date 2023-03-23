#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <SDL2/SDL.h>

#define PIXEL_SIZE 10
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define FONT_ADDRESS 0x50
#define START_ADDRESS 0x200

typedef struct {
    char graphics_memory[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint16_t opcode;
    uint8_t memory[4096];
    uint16_t pc;
    uint16_t index;
    uint8_t V[16];
    uint8_t delay_timer;
    uint8_t sp;
    uint16_t stack[16];
    uint8_t flag;
    bool pressed_key[16];
} Chip8;
extern Chip8 chip8;
extern SDL_Rect pixel_rect;

uint8_t get_0x00(uint16_t opcode);
uint8_t get_00x0(uint16_t opcode);
uint8_t get_00xx(uint16_t opcode);
uint16_t get_0xxx(uint16_t opcode);

void chip8_init();
int chip8_load_rom(char *file);
void chip8_forward();
void chip8_backward();
void chip8_render(SDL_Renderer *renderer);

#endif //CHIP8_H
