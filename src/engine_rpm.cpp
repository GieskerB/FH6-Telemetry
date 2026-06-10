#include <cstdio>
#include <stdlib.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <iostream>

#include "../include/engine_rpm.hpp"
#include "../include/colors.hpp"
namespace engine_rpm {

    static constexpr unsigned short WIDTH = 400;
    static constexpr unsigned short HEIGHT = 200;

    static constexpr unsigned short STEP_SIZE = 1000;

    static constexpr float RPM_FADE = 0.4f;

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
        font = TTF_OpenFont("assets/fonts/digital-7.ttf",255);
        if(font == nullptr) {
            perror(SDL_GetError());
            exit(EXIT_FAILURE);
        }
    }

    static void gear_texture(int gear) {
        static SDL_Texture* cached_gear_tex = nullptr;
        static int last_gear = gear;
        if (!cached_gear_tex || last_gear != gear) {
            if (cached_gear_tex) SDL_DestroyTexture(cached_gear_tex);

            char buffer[3]{0};
            switch (gear) {
                case 0:  SDL_snprintf(buffer, sizeof(buffer), " R"); break;
                case 11: SDL_snprintf(buffer, sizeof(buffer), " N"); break;
                default: SDL_snprintf(buffer, sizeof(buffer), "%2d", gear); break;
            }

            SDL_Surface* surf = TTF_RenderText_Blended(font, buffer, 0, ORANGE);
            if (surf) {
                cached_gear_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
                last_gear = gear;
            }
        }
        if (cached_gear_tex) {
            static const SDL_FRect gear_rect = { WIDTH * 0.75f, HEIGHT * 0.1f, WIDTH * 0.225f, HEIGHT * 0.4f };
            SDL_RenderTexture(renderer, cached_gear_tex, nullptr, &gear_rect);
        }
    }

    static void speed_texture(int speed) {
        const int speed_kmh = std::clamp(static_cast<int>(std::abs(speed * 3.6f)), 0, 999);
        static SDL_Texture* cached_speed_tex = nullptr;
        static int last_speed = speed_kmh;
        if (!cached_speed_tex ||last_speed != speed_kmh) {
            if (cached_speed_tex) SDL_DestroyTexture(cached_speed_tex);

            char buffer[4]{0};
            SDL_snprintf(buffer, sizeof(buffer), "%03d", speed_kmh);

            SDL_Surface* surf = TTF_RenderText_Blended(font, buffer, 0, WHITE);
            if (surf) {
                cached_speed_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
                last_speed = speed_kmh;
            }
        }
        if (cached_speed_tex) {
            static const SDL_FRect speed_rect = { HEIGHT * 0.1f, 0.0f, WIDTH * 0.667f, HEIGHT * 0.8f };
            SDL_RenderTexture(renderer, cached_speed_tex, nullptr, &speed_rect);
        }
    }

    static void static_texture() {
        static SDL_Texture* kmh_tex = nullptr;
        if (!kmh_tex) {
            SDL_Surface* surf = TTF_RenderText_Blended(font, "KM/H", 0, WHITE);
            if (surf) {
                kmh_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
            }
        }
        if (kmh_tex) {
            static const SDL_FRect unit_rect = { WIDTH * 0.75f, HEIGHT * 0.5f, WIDTH * 0.225f, HEIGHT * 0.2f };
            SDL_RenderTexture(renderer, kmh_tex, nullptr, &unit_rect);
        }
    }

    static void rpm_bar(float idle_rpm, float current_rpm, float max_rpm) {
        static  const SDL_FRect full_bar = {WIDTH * 0.05f, HEIGHT * 0.75f, WIDTH * 0.9f, WIDTH * 0.1f};

        // Draw the Background
        SDL_SetRenderDrawColor(renderer, GRAY.r, GRAY.g, GRAY.b, 69);
        SDL_RenderFillRect(renderer, &full_bar);

        // 2. Draw the Gradient Filled Bar
        const float rpm_percentage = std::clamp(current_rpm / max_rpm, 0.f,1.f);

        const float fill_width = full_bar.w * rpm_percentage;

        unsigned char r = 0, g = 0;

        if (rpm_percentage < RPM_FADE) {
            // Fade from Green (0, 255, 0) to Yellow (255, 255, 0)
            const float factor = rpm_percentage / RPM_FADE;
            r = static_cast<unsigned char>(factor * 255.0f);
            g = 255;
        } else {
            // Fade from Yellow (255, 255, 0) to Red (255, 0, 0)
            const float factor = (rpm_percentage - RPM_FADE) / (1-RPM_FADE);
            r = 255;
            g = static_cast<unsigned char>((1.0f - factor) * 255.0f);
        }

        SDL_SetRenderDrawColor(renderer, r, g, 0, 255);
        SDL_FRect fill_bar = {full_bar.x, full_bar.y, fill_width, full_bar.h};
        SDL_RenderFillRect(renderer, &fill_bar);


        // RPM Markers (1k, 2k, 3k...)
        const float pixels_per_rpm = full_bar.w / max_rpm;
        SDL_SetRenderDrawColor(renderer, LIGHT_GRAY.r, LIGHT_GRAY.g, LIGHT_GRAY.b, 255);
        for (int rpm_mark = STEP_SIZE; rpm_mark < max_rpm; rpm_mark += STEP_SIZE) {
            float mark_x = full_bar.x + (rpm_mark * pixels_per_rpm);
            SDL_RenderLine(renderer, mark_x, full_bar.y, mark_x, full_bar.y + (full_bar.h * 0.333f));
        }

        // Idle RPM Marker
        if (idle_rpm > 0.0f && idle_rpm < max_rpm) {
            float idle_x = full_bar.x + (idle_rpm * pixels_per_rpm);
            SDL_SetRenderDrawColor(renderer, BLUE.r, BLUE.g, BLUE.b, 255);
            SDL_RenderLine(renderer, idle_x, full_bar.y, idle_x, full_bar.y + full_bar.h);
        }

        // 5. Draw the Outer Border Outline (White)
        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, 255);
        SDL_RenderRect(renderer, &full_bar);

    }

    void update(const fh6_data & data_out) {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        // Render all the information
        static_texture();
        gear_texture(data_out.Gear);
        speed_texture(data_out.Speed);
        rpm_bar(data_out.EngineIdleRpm, data_out.CurrentEngineRpm, data_out.EngineMaxRpm);
        SDL_RenderPresent(renderer);
    }

    void close() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
    }
}
