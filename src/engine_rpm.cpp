#include <cstdio>
#include <stdlib.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <algorithm>

#include "../include/engine_rpm.hpp"
namespace engine_rpm {

    static constexpr short WIDTH = 400;
    static constexpr short HEIGHT = 200;

    static constexpr float RED_LINE_PERCENT = 0.9f;

    static constexpr SDL_Color ORANGE = {255, 127, 0, 255};
    static constexpr SDL_Color WHITE = {200, 200, 200, 255};

    static SDL_Window * window = nullptr;
    static SDL_Renderer* renderer = nullptr;
    static TTF_Font* font = nullptr;

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
        font = TTF_OpenFont("assets/digital-7.ttf",255);
        if(font == nullptr) {
            perror(SDL_GetError());
            exit(EXIT_FAILURE);
        }
    }

    static void gear_texture(const char gear_buffer[]) {
        static SDL_Texture* cached_gear_tex = nullptr;
        static char last_gear_str[3]{0};
        if (!cached_gear_tex || SDL_strcmp(last_gear_str, gear_buffer) != 0) {
            if (cached_gear_tex) SDL_DestroyTexture(cached_gear_tex);

            SDL_Surface* surf = TTF_RenderText_Blended(font, gear_buffer, 0, ORANGE);
            if (surf) {
                cached_gear_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
                SDL_strlcpy(last_gear_str, gear_buffer, sizeof(last_gear_str));
            }
        }
        if (cached_gear_tex) {
            static const SDL_FRect gear_rect = { WIDTH * 0.75f, HEIGHT * 0.1f, WIDTH * 0.225f, HEIGHT * 0.4f };
            SDL_RenderTexture(renderer, cached_gear_tex, nullptr, &gear_rect);
        }
    }

    static void speed_texture(const char speed_buffer[]) {
        static SDL_Texture* cached_speed_tex = nullptr;
        static char last_speed_str[3]{0};
        if (!cached_speed_tex || SDL_strcmp(last_speed_str, speed_buffer) != 0) {
            if (cached_speed_tex) SDL_DestroyTexture(cached_speed_tex);

            SDL_Surface* surf = TTF_RenderText_Blended(font, speed_buffer, 0, WHITE);
            if (surf) {
                cached_speed_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
                SDL_strlcpy(last_speed_str, speed_buffer, sizeof(last_speed_str));
            }
        }
        if (cached_speed_tex) {
            static const SDL_FRect speed_rect = { HEIGHT * 0.1f, 0.0f, WIDTH * 0.667f, HEIGHT * 0.8f };
            SDL_RenderTexture(renderer, cached_speed_tex, nullptr, &speed_rect);
        }
    }

    static void static_texture() {
        static SDL_Texture* static_kmh_tex = nullptr;
        if (!static_kmh_tex) {
            SDL_Surface* surf = TTF_RenderText_Blended(font, "KM/H", 0, WHITE);
            if (surf) {
                static_kmh_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
            }
        }
        if (static_kmh_tex) {
            static const SDL_FRect unit_rect = { WIDTH * 0.75f, HEIGHT * 0.5f, WIDTH * 0.225f, HEIGHT * 0.2f };
            SDL_RenderTexture(renderer, static_kmh_tex, nullptr, &unit_rect);
        }
    }

    static void rpm_bar(const float rpm_pct) {
        const float bar_x = WIDTH * 0.05f;
        const float bar_y = HEIGHT * 0.75f;
        const float bar_max_width = WIDTH * 0.9f;
        const float bar_height = WIDTH * 0.1f;
        const float current_bar_width = bar_max_width * rpm_pct;

        // Background track
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 128);
        SDL_FRect bg_bar = { bar_x, bar_y, bar_max_width, bar_height };
        SDL_RenderFillRect(renderer, &bg_bar);

        // Main Progress Bar
        SDL_SetRenderDrawColor(renderer, 220, 225, 230, 255);
        SDL_FRect progress_bar = { bar_x, bar_y, current_bar_width, bar_height };
        SDL_RenderFillRect(renderer, &progress_bar);

        // Redline indicator
        if (rpm_pct > RED_LINE_PERCENT) {
            float redline_start_x = bar_x + (bar_max_width * RED_LINE_PERCENT);
            float redline_width = current_bar_width - (bar_max_width * RED_LINE_PERCENT);

            if (redline_width > 0.0f) {
                SDL_SetRenderDrawColor(renderer, 255, 42, 109, 255);
                SDL_FRect redline_bar = { redline_start_x, bar_y, redline_width, bar_height };
                SDL_RenderFillRect(renderer, &redline_bar);
            }
        }
    }

    void update(const fh6_data & data_out) {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 69, 69, 69, 69);
        SDL_RenderClear(renderer);

        // Calculate relevant data form data_out.
        const int speed_kmh = std::clamp(static_cast<int>(std::abs(data_out.VelocityZ * 3.6f)), 0, 999);
        const float rpm_pct = std::clamp(data_out.CurrentEngineRpm / data_out.EngineMaxRpm, 0.0f, 1.0f);

        char gear_buffer[3]{0};
        switch (data_out.Gear) {
            case 0:  SDL_snprintf(gear_buffer, sizeof(gear_buffer), " R"); break;
            case 11: SDL_snprintf(gear_buffer, sizeof(gear_buffer), " N"); break;
            default: SDL_snprintf(gear_buffer, sizeof(gear_buffer), "%2d", data_out.Gear); break;
        }

        char speed_buffer[4]{0};
        SDL_snprintf(speed_buffer, sizeof(speed_buffer), "%03d", speed_kmh);

        // Render all the information
        gear_texture(gear_buffer);
        speed_texture(speed_buffer);
        static_texture();
        rpm_bar(rpm_pct);
        SDL_RenderPresent(renderer);
    }

    void close() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
    }
}
