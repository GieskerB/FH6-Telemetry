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
    static constexpr int G_MAX = 4;
    static constexpr int SPEED_MAX = G_MAX * 100;

    static SDL_Window * window = nullptr;
    static SDL_Renderer* renderer = nullptr;
    static TTF_Font* font = nullptr;

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

    static void draw_points(short head, const SDL_Point point, SDL_Point hist[], const SDL_Color& color) {
        hist[head] = point;
        int index = head--;
        for(int i = 0; i < HISTORY_SIZE; ++i) {
            auto & p = hist[index++];
            if(index >= HISTORY_SIZE) index = 0;;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255 - i * (255.f / HISTORY_SIZE));
            const SDL_FRect rect {p.x -3.f , p.y - 3.f,5,5};
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    static void draw_points(const SDL_Point& gforce_point, const SDL_Point& speed_point) {
        static short head = 0;
        static SDL_Point g_history[HISTORY_SIZE]{0,0};
        static SDL_Point s_history[HISTORY_SIZE]{0,0};

        draw_points(head, gforce_point, g_history, ORANGE);
        draw_points(head, speed_point, s_history, BLUE);

        if(--head < 0) head = HISTORY_SIZE-1;
    }

    static void draw_background() {
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
    }

    static void draw_labels() {
        static SDL_Texture* gforce_labels[G_MAX];
        if (!gforce_labels[0]) {
            for(int i = 0 ; i < G_MAX; ++i) {
                // for each ring of the the gforce meter
                char buffer[3];
                SDL_snprintf(buffer, sizeof(buffer), "%dG", i+1);
                SDL_Surface* surf = TTF_RenderText_Blended(font, buffer, 0, ORANGE);
                if (surf) {
                    gforce_labels[i] = SDL_CreateTextureFromSurface(renderer, surf);
                    SDL_DestroySurface(surf);
                }
            }
        }
        if (gforce_labels[0]) {
            for(int i = 0 ; i < G_MAX; ++i) {
                const SDL_FRect unit_rect = { WIDTH /2.f + (i+0.5f) * WIDTH / 8.f, HEIGHT / 2, WIDTH * 0.05f, WIDTH * 0.05f };
                SDL_RenderTexture(renderer, gforce_labels[i], nullptr, &unit_rect);
            }
        }


        static SDL_Texture* speed_labels[G_MAX];
        if (!speed_labels[0]) {
            for(int i = 0 ; i < G_MAX; ++i) {
                // for each ring of the the speed meter
                char buffer[G_MAX]{0};
                SDL_snprintf(buffer, sizeof(buffer), "%d", SPEED_MAX - i * 100);
                SDL_Surface* surf = TTF_RenderText_Blended(font, buffer, 0, BLUE);
                if (surf) {
                    speed_labels[i] = SDL_CreateTextureFromSurface(renderer, surf);
                    SDL_DestroySurface(surf);
                }
            }
        }
        if (speed_labels[0]) {
            for(int i = 0 ; i < G_MAX; ++i) {
                const SDL_FRect unit_rect = { (i) * WIDTH / 8.f, HEIGHT / 2, WIDTH * 0.075f, WIDTH * 0.05f };
                SDL_RenderTexture(renderer, speed_labels[i], nullptr, &unit_rect);
            }
        }
    }

    static void draw_gforce(float gforce) {
        static SDL_Texture* cached_gforce_tex = nullptr;
        static float last_gforce = gforce;
        if (!cached_gforce_tex or last_gforce != gforce) {
            if (cached_gforce_tex) SDL_DestroyTexture(cached_gforce_tex);

            char buffer[6]{0};
            SDL_snprintf(buffer, sizeof(buffer), "%.2fG", std::clamp(gforce,0.f,9.99f));

            SDL_Surface* surf = TTF_RenderText_Blended(font, buffer, 0, ORANGE);
            if (surf) {
                cached_gforce_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
                last_gforce = gforce;
            }
        }
        if (cached_gforce_tex) {
            const SDL_FRect unit_rect = { WIDTH * 0.7f, HEIGHT * 0.92f, WIDTH * 0.3f, WIDTH * 0.08f };
            SDL_RenderTexture(renderer, cached_gforce_tex, nullptr, &unit_rect);
        }
    }

    static void draw_speed(float speed) {
        static SDL_Texture* cached_speed_tex = nullptr;
        static float last_speed = speed;
        if (!cached_speed_tex or last_speed != speed) {
            if (cached_speed_tex) SDL_DestroyTexture(cached_speed_tex);

            char buffer[8]{0};
            SDL_snprintf(buffer, sizeof(buffer), "%03dKM/H", std::clamp(static_cast<int>(speed),0,999));

            SDL_Surface* surf = TTF_RenderText_Blended(font, buffer, 0, BLUE);
            if (surf) {
                cached_speed_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
                last_speed = speed;
            }
        }
        if (cached_speed_tex) {
            const SDL_FRect unit_rect = { 0, HEIGHT * 0.92f, WIDTH * 0.3f, WIDTH * 0.08f };
            SDL_RenderTexture(renderer, cached_speed_tex, nullptr, &unit_rect);
        }
    }
    void update(const fh6_data& data_out) {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        draw_background();
        draw_labels();

        float z = data_out.AccelerationZ / 9.81f;
        float x = -data_out.AccelerationX / 9.81f;
        float combined = std::sqrt(z * z + x * x);

        if(combined > G_MAX) {
            z = z / combined * G_MAX;
            x = x / combined * G_MAX;
        }

        const SDL_Point gforce_point {WIDTH /2 + static_cast<int>((x / G_MAX) * WIDTH /2), HEIGHT /2 + static_cast<int>((z / G_MAX) * HEIGHT /2)};
        draw_gforce(combined);
        
        z = -data_out.VelocityZ * 3.6f;
        x = data_out.VelocityX * 3.6f;
        combined = std::sqrt(z * z + x * x);

        if(combined > G_MAX * 100) {
            z = z / combined * SPEED_MAX;
            x = x / combined * SPEED_MAX;
        }

        const SDL_Point speed_point {WIDTH /2 + static_cast<int>((x / SPEED_MAX) * WIDTH /2),HEIGHT /2 + static_cast<int>((z / SPEED_MAX) * HEIGHT /2)};

        draw_speed(combined);
        draw_points(gforce_point, speed_point);

        SDL_RenderPresent(renderer);
    }

    void close() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
    }

}
