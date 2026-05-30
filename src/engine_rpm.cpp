#include <cstdio>
#include <stdlib.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "../include/engine_rpm.hpp"
#include <iostream>
#include <algorithm>

namespace engine_rpm {

    static constexpr short WIDTH = 400;
    static constexpr short HEIGHT = 200;

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
        font = TTF_OpenFont("assets/airstrike.ttf",255);
        if(font == nullptr) {
            perror(SDL_GetError());
            exit(EXIT_FAILURE);
        }
    }

void update(const fh6_data & data_out) {
        // 1. Clear the screen with a transparent background
        SDL_SetRenderDrawColor(renderer, 69, 69, 69, 69);
        SDL_RenderClear(renderer);

        // ==========================================
        // 2. DATA CALCULATIONS
        // ==========================================
        // Convert VelocityZ (m/s) to kmh
        int speed_kmh = std::abs(static_cast<int>(data_out.VelocityZ * 3.6));
        speed_kmh = std::clamp(speed_kmh, 0, 999);
        
        // Calculate RPM percentage for the bar
        float rpm_range = data_out.EngineMaxRpm - data_out.EngineIdleRpm;
        float rpm_pct = 0.0f;
        if (rpm_range > 0.0f) {
            rpm_pct = (data_out.CurrentEngineRpm - data_out.EngineIdleRpm) / rpm_range;
            rpm_pct = std::clamp(rpm_pct, 0.0f, 1.0f);
        }

        // Format Strings
        std::string gear_str = (data_out.Gear == 0) ? "N" : std::to_string(data_out.Gear);
        

        // Render Gear Text using SDL3_ttf
        SDL_Color color = {255, 127, 0, 255};
        SDL_Surface* gear_surface = TTF_RenderText_Blended(font, gear_str.c_str(), 0, color);
        if (gear_surface) {
            SDL_Texture* gear_texture = SDL_CreateTextureFromSurface(renderer, gear_surface);
            if (gear_texture) {
                SDL_FRect gear_rect = {
                    WIDTH * 0.75 ,
                    HEIGHT * 0.1 ,
                    WIDTH * 0.2,
                    HEIGHT * 0.4
                };
                SDL_RenderTexture(renderer, gear_texture, nullptr, &gear_rect);
                SDL_DestroyTexture(gear_texture);
            }
            SDL_DestroySurface(gear_surface);
        }

        // ==========================================
        // 4. DRAW SPEEDOMETER TEXT (000)
        // ==========================================
        // Format speed with leading zeros (e.g., "000")
        char speed_buffer[4];
        snprintf(speed_buffer, sizeof(speed_buffer), "%03d", speed_kmh);

        SDL_Color white_muted = {200, 200, 200, 255};
        SDL_Surface* speed_surface = TTF_RenderText_Blended(font, speed_buffer, 0, white_muted);
        if (speed_surface) {
            SDL_Texture* speed_texture = SDL_CreateTextureFromSurface(renderer, speed_surface);
            if (speed_texture) {
                // Positioned to the right of the gear circle, scaled up
                SDL_FRect speed_rect = {
                    HEIGHT * 0.1f,
                    HEIGHT * 0,
                    WIDTH * 0.667,
                    HEIGHT * 0.8f };
                SDL_RenderTexture(renderer, speed_texture, nullptr, &speed_rect);
                SDL_DestroyTexture(speed_texture);
            }
            SDL_DestroySurface(speed_surface);
        }

        // Render auxiliary labels ("kmh")
        SDL_Surface* unit_surface = TTF_RenderText_Blended(font, "kmh", 0, white_muted);
        if (unit_surface) {
            SDL_Texture* unit_texture = SDL_CreateTextureFromSurface(renderer, unit_surface);
            if (unit_texture) {
                SDL_FRect unit_rect = {
                    WIDTH * 0.75 ,
                    HEIGHT * 0.5 ,
                    WIDTH * 0.2,
                    HEIGHT * 0.2
                };
                SDL_RenderTexture(renderer, unit_texture, nullptr, &unit_rect);
                SDL_DestroyTexture(unit_texture);
            }
            SDL_DestroySurface(unit_surface);
        }

        // ==========================================
        // 5. DRAW THE REV COUNTER BAR
        // ==========================================
        float bar_x = WIDTH * 0.1;
        float bar_y = HEIGHT * 0.75;
        float bar_max_width = WIDTH * 0.8f;
        float bar_height = WIDTH / 20;
        float current_bar_width = bar_max_width * rpm_pct;

        // Background track (Dark semi-transparent tray)
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 128);
        SDL_FRect bg_bar = { bar_x, bar_y, bar_max_width, bar_height };
        SDL_RenderFillRect(renderer, &bg_bar);

        // Main Progress Bar (Light silver/white)
        SDL_SetRenderDrawColor(renderer, 220, 225, 230, 255);
        SDL_FRect progress_bar = { bar_x, bar_y, current_bar_width, bar_height };
        SDL_RenderFillRect(renderer, &progress_bar);

        // Redline indicator (If RPM is over 90%, paint the active tip neon pink/red)
        if (rpm_pct > 0.90f) {
            float redline_start_x = bar_x + (bar_max_width * 0.90f);
            float redline_width = current_bar_width - (bar_max_width * 0.90f);
            
            if (redline_width > 0.0f) {
                SDL_SetRenderDrawColor(renderer, 255, 42, 109, 255); 
                SDL_FRect redline_bar = { redline_start_x, bar_y, redline_width, bar_height };
                SDL_RenderFillRect(renderer, &redline_bar);
            }
        }

        // Final screen presentation for SDL 3
        SDL_RenderPresent(renderer);
    }
    
    // void update(const fh6_data & data_out) {
    //     // data_out.CurrentEngineRpm
    //     // data_out.EngineIdleRpm
    //     // data_out.EngineMaxRpm
    //     // data_out.Gear
    //     // data_out.VelocityZ
    // }

    void close() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
    }
}

