#ifndef CHIP8_LOG_H
#define CHIP8_LOG_H

#include <stdbool.h>
#include "chip8.h"

#define INSTRUCTION_SIZE 0xff

#define GREY "\033[90m"
#define YELLOW "\033[93m"
#define RESET "\033[0m"

extern bool logging;
void chip8_logging(Chip8 *chip8);
void chip8_logging_init();
void chip8_logging_end();

#endif //CHIP8_LOG_H
