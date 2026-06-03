#include <cstdio>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <SDL3/SDL_surface.h>

#include "../include/gforce.hpp"

namespace gforce {

    static constexpr short WIDTH = 200;
    static constexpr short HEIGHT = 200;

    static constexpr SDL_Color ORANGE = {255, 127, 0, 255};
    static constexpr SDL_Color WHITE = {200, 200, 200, 255};

    static SDL_Window * window = nullptr;
    static SDL_Renderer* renderer = nullptr;

    void init() {
        window = SDL_CreateWindow("Engine RPM",WIDTH,HEIGHT,SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
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

    static void static_texture() {
            static SDL_Texture* static_gforce_tex = nullptr;
        if (!static_gforce_tex) {
            SDL_Surface* surf = SDL_LoadPNG("assets/gforce_background_trans.png");
            if (surf) {
                static_gforce_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
            }
        }
        if (static_gforce_tex) {
            static const SDL_FRect unit_rect = { 0,0,WIDTH,HEIGHT };
            SDL_RenderTexture(renderer, static_gforce_tex, nullptr, &unit_rect);
        }
    }

    void update(const fh6_data& data_out) {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 69, 69, 69, 69);
        SDL_RenderClear(renderer);

        const float gforce_x = std::clamp(-data_out.AccelerationX / 9.81f, -3.f, 3.f);
        const float gforce_z = std::clamp(-data_out.AccelerationX / 9.81f, -3.f, 3.f);

        std::cout << gforce_x << " " << gforce_z << "\n";

        static_texture();

        SDL_RenderPresent(renderer);
    }

    void close() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }

}
