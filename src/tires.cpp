#include <cstdio>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <algorithm>

#include <iostream>
#include <cmath>
#include <vector>

#include "../include/tires.hpp"

namespace tires{

    static constexpr unsigned short WIDTH = 300;
    static constexpr unsigned short HEIGHT = 300;

    static SDL_Window * window = nullptr;
    static SDL_Renderer* renderer = nullptr;

    void init() {
        window = SDL_CreateWindow("GForce",WIDTH,HEIGHT,SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
        if(window == nullptr) {
            perror(SDL_GetError());
            exit(EXIT_FAILURE);
        }
        renderer = SDL_CreateRenderer(window, NULL);
        if(renderer == nullptr) {
            perror(SDL_GetError());
            exit(EXIT_FAILURE);
        }
    }

    static void draw_tire(float temperature) {
        if (temperature > 0) asm("nop");
    }

    void update(const fh6_data& data_out) {
        SDL_SetRenderDrawColor(renderer, 15, 15, 20, 255); 
        SDL_RenderClear(renderer);
        draw_tire(data_out.TireTempFrontLeft);
        SDL_RenderPresent(renderer);
    }

    void close() {

    }

}