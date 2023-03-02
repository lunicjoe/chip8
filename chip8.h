#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H

#include <SDL2/SDL.h>

#define PIXEL_SIZE 5
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

typedef struct {
    char graphics_memory[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint16_t opcode;
    uint8_t memory[4096];
    uint8_t chip8_V[16];
    uint8_t index;
    uint16_t pc;
} Chip8;
extern Chip8 chip8;
extern SDL_Rect rectangle;
void chip8_init();
void chip8_load_rom(char *file);
void chip8_cycle();
void chip8_clear();
void chip8_draw(SDL_Renderer *renderer);

#endif //CHIP8_CHIP8_H
