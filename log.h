//
// Created by filadelphila on 7/03/23.
//

#ifndef CHIP8_LOG_H
#define CHIP8_LOG_H

#include <stdbool.h>
#include "emulator/chip8.h"

#define INSTRUCTION_SIZE 0xff
#define bytecode_log(text, ...) snprintf(instruction, INSTRUCTION_SIZE, text, ##__VA_ARGS__)

#define GREY "\033[90m"
#define YELLOW "\033[93m"
#define RESET "\033[0m"

extern bool logging;
void chip8_log(Chip8 *chip8, uint16_t address, char *instruction);

#endif //CHIP8_LOG_H
