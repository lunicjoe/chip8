#include "chip8.h"

#include <SDL2/SDL.h>

SDL_Rect rectangle;
Chip8 chip8;

void chip8_init() {
    SDL_RectEmpty(&rectangle);
    rectangle.h = PIXEL_SIZE;
    rectangle.w = PIXEL_SIZE;
    chip8_clear();
    chip8.opcode = 0;
    chip8.pc = 0x200;
}

void chip8_load_rom(char *file) {
    FILE *rom = fopen(file, "rb");
    fseek(rom, 0, SEEK_END);
    const long rom_size = ftell(rom);
    rewind(rom);
    uint16_t *rom_buffer = (uint16_t*) malloc(sizeof(uint16_t) * rom_size);
    fread(rom_buffer, sizeof(uint16_t), rom_size, rom);
    fclose(rom);
    for (int i = 0; i < rom_size; i++) {
        chip8.chip8_memory[0x200 + i] = (rom_buffer[i]>>8) | (rom_buffer[i]<<8);
    }
}

void chip8_cycle() {
    char instruction[100] = "";
    chip8.opcode = chip8.chip8_memory[chip8.pc];
    if (chip8.opcode == 0x00e0) {
        strncpy(instruction, "cls", 100);
        chip8_clear();
    } else if ((chip8.opcode & 0xf000) == 0x1000) {
        snprintf(instruction, 100, "jump 0x%X", chip8.opcode & 0xfff);
        chip8.pc = chip8.opcode & 0xfff - 1;
    } else if ((chip8.opcode & 0xf000) == 0x6000) {
        snprintf(instruction, 100, "mov V%d, 0x%X", chip8.opcode & 0x0f00, chip8.opcode & 0x00ff);
        chip8.chip8_V[chip8.opcode & 0x0f00] = chip8.opcode & 0x00ff;
    } else if ((chip8.opcode & 0xf000) == 0x7000) {
        snprintf(instruction, 100, "add r%d, 0x%X", chip8.opcode & 0x0f00, chip8.opcode & 0x00ff);
        chip8.chip8_V[chip8.opcode & 0x0f00] += chip8.opcode & 0x00ff;
    } else if ((chip8.opcode & 0xf000) == 0xa000) {
        snprintf(instruction, 100, "mvi 0x%X", chip8.opcode & 0x0fff);
        chip8.index = chip8.opcode & 0x0fff;
    } else if ((chip8.opcode & 0xf000) == 0xd000) {
        snprintf(instruction, 100, "draw %d, %d", chip8.opcode & 0x0f00, chip8.opcode & 0x00f0);
        chip8.graphics_memory[chip8.opcode & 0x0f00 + SCREEN_WIDTH * chip8.opcode & 0x00f0] = 1;
    }
    printf("0x%04x -> %04x\t%s\n", chip8.pc, chip8.chip8_memory[chip8.pc], instruction);
    chip8.pc++;
}

void chip8_clear() {
    for (int pixel = 0; pixel < SCREEN_WIDTH * SCREEN_HEIGHT; pixel++) {
        chip8.graphics_memory[pixel] = 0;
    }
}

void chip8_draw(SDL_Renderer *renderer) {
    for (int pixel = 0; pixel < SCREEN_WIDTH * SCREEN_HEIGHT; pixel++) {
        rectangle.x = pixel % SCREEN_WIDTH * PIXEL_SIZE;
        rectangle.y = (pixel - (pixel % SCREEN_WIDTH)) / SCREEN_WIDTH * PIXEL_SIZE;
        if (chip8.graphics_memory[pixel] == 1)
            SDL_RenderFillRect(renderer, &rectangle);
    }
}
