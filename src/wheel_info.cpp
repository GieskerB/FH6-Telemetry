#include "../include/wheel_info.hpp"

#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "../include/util/colors.hpp"
#include "../include/util/texture_handler.hpp"

static unsigned short WIDTH;
static unsigned short HEIGHT;

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static TTF_Font* font = nullptr;

void wheel_info_t::init(unsigned short size) {
    WIDTH = size;
    HEIGHT = static_cast<unsigned short>(size * 1.f);

    window = SDL_CreateWindow("Race Info", WIDTH, HEIGHT, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
    if (window == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    font = TTF_OpenFont("assets/fonts/droid-sans.ttf", 100);
    if (font == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

static std::array<SDL_Color, 4> update_temperature(float temps[4], unsigned short& changed) {
    static float last_temps[4]{-1};
    static std::array<SDL_Color,4> return_value{};
    for (unsigned char i = 0; i < 4; ++i) {
        if (temps[i] != last_temps[i]) {
            last_temps[i] = temps[i];
            changed |= (0b1 << i);
        }
    }
    return return_value;
}

void wheel_info_t::update(const fh6_data& data_out) {}

void wheel_info_t::render() {}

void wheel_info_t::close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
}
