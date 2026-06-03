#include <cstdio>
#include <stdlib.h>
#include <algorithm>
#include <SDL3/SDL_surface.h>

#include "../include/gforce.hpp"
#include <cmath>

namespace gforce {

    static constexpr unsigned short WIDTH = 300;
    static constexpr unsigned short HEIGHT = 300;
    static constexpr unsigned char HISTORY_SIZE = 255;
    static constexpr float G_MAX = 5.f;

    static constexpr SDL_Color ORANGE = {255, 127, 0, 255};
    static constexpr SDL_Color WHITE = {200, 200, 200, 255};

    static SDL_Window * window = nullptr;
    static SDL_Renderer* renderer = nullptr;

    static SDL_Point history[HISTORY_SIZE]{0,0};

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
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }

    static void draw_point(const SDL_Point& point) {
        static unsigned short head = 0;
        history[head] = point;

        int index = head++;
        if(head >= HISTORY_SIZE) head = 0;
        for(int i = 0; i < HISTORY_SIZE; ++i) {
            auto & p = history[index++];
            if(index >= HISTORY_SIZE) index = 0;
            SDL_SetRenderDrawColor(renderer, 255, 127, 0, i - 255);
            const SDL_FRect rect {p.x -2.f , p.y - 2.f,5,5};
            SDL_RenderFillRect(renderer, &rect);
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

        float gforce_z = -data_out.AccelerationZ / 9.81f;
        float gforce_x = -data_out.AccelerationX / 9.81f;
        const float gforce_total = std::sqrt(gforce_z * gforce_z + gforce_x * gforce_x);

        if(gforce_total > G_MAX) {
            gforce_z = gforce_z / gforce_total * G_MAX;
            gforce_x = gforce_x / gforce_total * G_MAX;
        }

        SDL_Point gforce_point;
        gforce_point.x = WIDTH /2 + static_cast<int>((gforce_x / G_MAX) * WIDTH /2);
        gforce_point.y =  HEIGHT /2 + static_cast<int>((gforce_z / G_MAX) * HEIGHT /2);

        static_texture();
        draw_point(gforce_point);

        SDL_RenderPresent(renderer);
    }

    void close() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }

}
