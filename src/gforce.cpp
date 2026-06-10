#include <cstdio>
#include <stdlib.h>
#include <algorithm>
#include <cmath>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "../include/gforce.hpp"
#include "../include/colors.hpp"
namespace gforce {

    static constexpr unsigned short WIDTH = 300;
    static constexpr unsigned short HEIGHT = 300;
    
    static constexpr unsigned char HISTORY_SIZE = 69;
    static constexpr float G_MAX = 4.f;

    static SDL_Window * window = nullptr;
    static SDL_Renderer* renderer = nullptr;
    static TTF_Font* font = nullptr;

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
        font = TTF_OpenFont("assets/fonts/digital-7.ttf",255);
        if(font == nullptr) {
            perror(SDL_GetError());
            exit(EXIT_FAILURE);
        }
    }

    static void draw_points(const SDL_Point& point) {
        static short head = 0;
        history[head] = point;

        int index = head--;
        if(head < 0) head = HISTORY_SIZE-1;
        for(int i = 0; i < HISTORY_SIZE; ++i) {
            auto & p = history[index++];
            if(index >= HISTORY_SIZE) index = 0;;
            SDL_SetRenderDrawColor(renderer, 255, 127, 0, 255 - i * (255.f / HISTORY_SIZE));
            const SDL_FRect rect {p.x -3.f , p.y - 3.f,5,5};
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    static void draw_background_texture() {
        static SDL_Texture* background_tex = nullptr;
        if (!background_tex) {
            SDL_Surface* surf = SDL_LoadPNG("assets/sprites/gforce_background.png");
            if (surf) {
                background_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
            }
        }
        if (background_tex) {
            static const SDL_FRect unit_rect = { 0,0,WIDTH,HEIGHT };
            SDL_RenderTexture(renderer, background_tex, nullptr, &unit_rect);
        }

        static SDL_Texture* texts_tex[4];
        if (!texts_tex[0]) {
            for(int i = 0 ; i < 4; ++i) {
                // for each ring of the the gforce meter
                char buffer[3];
                SDL_snprintf(buffer, sizeof(buffer), "%dG", i);
                SDL_Surface* surf = TTF_RenderText_Blended(font, buffer, 0, ORANGE);
                if (surf) {
                    texts_tex[i] = SDL_CreateTextureFromSurface(renderer, surf);
                    SDL_DestroySurface(surf);
                }
            }

        }
        if (texts_tex[0]) {
            for(int i = 0 ; i < 4; ++i) {
                const SDL_FRect unit_rect = { WIDTH /2.f + (i+0.5f) * WIDTH / 8.f, HEIGHT / 2, WIDTH * 0.05f, WIDTH * 0.05f };
                SDL_RenderTexture(renderer, texts_tex[i], nullptr, &unit_rect);
            }
        }
    }

    static void draw_gforce(const char gforce_buffer[]) {
        static SDL_Texture* cached_gforce_tex = nullptr;
        static char last_gforce_str[3]{0};
        if (!cached_gforce_tex || SDL_strcmp(last_gforce_str, gforce_buffer) != 0) {
            if (cached_gforce_tex) SDL_DestroyTexture(cached_gforce_tex);

            SDL_Surface* surf = TTF_RenderText_Blended(font, gforce_buffer, 0, ORANGE);
            if (surf) {
                cached_gforce_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
                SDL_strlcpy(last_gforce_str, gforce_buffer, sizeof(last_gforce_str));
            }
        }
        if (cached_gforce_tex) {
            const SDL_FRect unit_rect = { WIDTH * 0.7f, HEIGHT * 0.92f, WIDTH * 0.3f, WIDTH * 0.08f };
            SDL_RenderTexture(renderer, cached_gforce_tex, nullptr, &unit_rect);
        }

    }

    void update(const fh6_data& data_out) {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        float gforce_z = data_out.AccelerationZ / 9.81f;
        float gforce_x = -data_out.AccelerationX / 9.81f;
        const float gforce_total = std::sqrt(gforce_z * gforce_z + gforce_x * gforce_x);

        if(gforce_total > G_MAX) {
            gforce_z = gforce_z / gforce_total * G_MAX;
            gforce_x = gforce_x / gforce_total * G_MAX;
        }

        SDL_Point gforce_point;
        gforce_point.x = WIDTH /2 + static_cast<int>((gforce_x / G_MAX) * WIDTH /2);
        gforce_point.y =  HEIGHT /2 + static_cast<int>((gforce_z / G_MAX) * HEIGHT /2);

        char gforce_buffer[6]{0};
        SDL_snprintf(gforce_buffer, sizeof(gforce_buffer), "%.2fG", std::clamp(gforce_total,0.f,9.99f));

        draw_background_texture();
        draw_gforce(gforce_buffer);
        draw_points(gforce_point);

        SDL_RenderPresent(renderer);
    }

    void close() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
    }

}
