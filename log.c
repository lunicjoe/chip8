#include "log.h"

#include <stdio.h>
#include "emulator/chip8.h"

bool logging = false;

void chip8_log(Chip8 *chip8, uint16_t address, char *instruction) {
    if (logging) {
        stdout = fopen("log", "a");
        printf("0x%04x: %s0x%04x %s%s %sPC:0x%02X I:0x%02X V(", address, GREY, chip8->opcode, YELLOW, instruction, GREY, chip8->pc, chip8->index);
        for (int v = 0; v < 16; v++) {
            printf("%X:%02X", v, chip8->V[v]);
            if (v < 15) printf(", ");
        }
        printf(") SP:0x%02X ", chip8->sp);
        for (int i = 0; i < chip8->sp; i++) {
            printf("stack[0x%X]:0x%02X ", i, chip8->stack[i]);
        }
        printf("\n%s", RESET);
    }
}