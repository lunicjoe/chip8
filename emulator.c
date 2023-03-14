#include <stdbool.h>
#include <SDL2/SDL.h>
#include "emulator/chip8.h"
#include "log.h"

bool debug = false;

int main(int argc, char *argv[]) {
    chip8_init();
    for (int arg = 1; arg < argc; arg++) {
        if (strcmp(argv[arg], "--log") == 0) {
            logging = true;
        } else if (strcmp(argv[arg], "--debug") == 0) {
            debug = true;
        } else if (!chip8_load_rom(argv[arg])) {
            fprintf(stderr, "rom %s not found\n", argv[arg]);
            return EXIT_FAILURE;
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
        if (debug) {
            if (keyboard_state_current[SDL_SCANCODE_SPACE]) {
                if (keyboard_state_current[SDL_SCANCODE_RIGHT]) chip8_cycle();
                if (keyboard_state_current[SDL_SCANCODE_LEFT]) chip8_backward();
            } else {
                if (!keyboard_state_current[SDL_SCANCODE_RIGHT] && keyboard_state_last[SDL_SCANCODE_RIGHT]) chip8_cycle();
                if (!keyboard_state_current[SDL_SCANCODE_LEFT] && keyboard_state_last[SDL_SCANCODE_LEFT]) chip8_backward();
            }
        } else {
            chip8_cycle();
        }
        for (int i = 0; i < numkeys; i++) {
            keyboard_state_last[i] = keyboard_state_current[i];
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        chip8_render(renderer);

        SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 0);
        SDL_RenderDrawRect(renderer, &rectangle);
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0);

        SDL_RenderPresent(renderer);
        SDL_Delay(20);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
