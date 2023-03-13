#include <stdbool.h>
#include <SDL2/SDL.h>
#include "emulator/chip8.h"
#include "log.h"

int main(int argc, char *argv[]) {
    chip8_init();
    for (int arg = 1; arg < argc; arg++) {
        if (strcmp(argv[arg], "--log") == 0) {
            logging = true;
        } else {
            if (!chip8_load_rom(argv[arg])) {
                fprintf(stderr, "rom %s not found\n", argv[arg]);
                return EXIT_FAILURE;
            }
        }
    }

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH * PIXEL_SIZE, SCREEN_HEIGHT * PIXEL_SIZE, SDL_WINDOW_RESIZABLE, &window, &renderer);

    bool is_running = true;
    int numkeys;
    const uint8_t *keyboard_state_current = SDL_GetKeyboardState(&numkeys);
    uint8_t *keyboard_state_last = malloc(numkeys);
    while (is_running) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) is_running = false;
        if (!keyboard_state_current[SDL_SCANCODE_RETURN] && keyboard_state_last[SDL_SCANCODE_RETURN]) {
            chip8_cycle();
        }
        if (keyboard_state_current[SDL_SCANCODE_SPACE]) chip8_cycle();
        for (int i = 0; i < numkeys; i++) {
            keyboard_state_last[i] = keyboard_state_current[i];
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        chip8_render(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(20);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
