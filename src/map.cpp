#include <cstdio>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <algorithm>

#include "../include/map.hpp"
#include <iostream>

namespace map{

    static constexpr unsigned short WIDTH = 780;
    static constexpr unsigned short HEIGHT = 996;

    static SDL_Window * window = nullptr;
    static SDL_Renderer* renderer = nullptr;

    void init() {
        window = SDL_CreateWindow("Map",WIDTH,HEIGHT,SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
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

    static void map_texture() {
        static SDL_Texture* static_map_tex = nullptr;
        if (!static_map_tex) {

            SDL_Surface* surf = SDL_LoadPNG("assets/maps/winter.png");
            if (surf) {
                static_map_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
            } 
        }
        if (static_map_tex) {
            static const SDL_FRect unit_rect = {0,0,WIDTH,HEIGHT };
            SDL_RenderTexture(renderer, static_map_tex, nullptr, &unit_rect);
        }
    }

    void update(const fh6_data& data_out) {
        SDL_SetRenderDrawColor(renderer, 15, 15, 20, 255); 
        SDL_RenderClear(renderer);

        float pos_x = data_out.PositionX;
        float pos_y = data_out.PositionZ;

        if(pos_x != pos_y)
        map_texture();

        SDL_RenderPresent(renderer);
    }

    void close() {

    }

}