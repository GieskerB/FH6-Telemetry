#include <cstdio>
#include <stdlib.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "../include/engine_rpm.hpp"
#include <iostream>
#include <cmath>
#include <vector>

namespace engine_rpm {

    static constexpr short WIDTH = 400;
    static constexpr short HEIGHT = 400;

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
        font = TTF_OpenFont("assets/airstrike.ttf",24);
        if(font == nullptr) {
            perror(SDL_GetError());
            exit(EXIT_FAILURE);
        }
    }

// High-performance procedural arc renderer using stack-allocated primitives
    void DrawArc(SDL_Renderer* ren, float cx, float cy, float r, float start_angle, float end_angle, float thickness, SDL_Color color) {
        if (start_angle > end_angle) return;

        // Drastically lowered segment count for background structures
        // Since the lines are narrow, 24 segments provide visual smoothness without CPU overhead
        constexpr int MAX_SEGMENTS = 24; 
        
        SDL_Vertex vertices[(MAX_SEGMENTS + 1) * 2];
        int indices[MAX_SEGMENTS * 6];

        float r_out = r + thickness / 2.0f;
        float r_in = r - thickness / 2.0f;
        SDL_FColor fcolor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

        for (int i = 0; i <= MAX_SEGMENTS; ++i) {
            float t = (float)i / MAX_SEGMENTS;
            float angle = start_angle + t * (end_angle - start_angle);
            float rad = angle * (M_PI / 180.0f);

            float c = std::cos(rad);
            float s = std::sin(rad);

            int v_idx = i * 2;
            vertices[v_idx]     = { { cx + r_out * c, cy + r_out * s }, fcolor, { 0, 0 } }; // Outer arc bound
            vertices[v_idx + 1] = { { cx + r_in * c,  cy + r_in * s  }, fcolor, { 0, 0 } }; // Inner arc bound

            if (i < MAX_SEGMENTS) {
                int base_idx = i * 6;
                indices[base_idx]     = v_idx;
                indices[base_idx + 1] = v_idx + 1;
                indices[base_idx + 2] = v_idx + 2;

                indices[base_idx + 3] = v_idx + 1;
                indices[base_idx + 4] = v_idx + 3;
                indices[base_idx + 5] = v_idx + 2;
            }
        }

        SDL_RenderGeometry(ren, nullptr, vertices, (MAX_SEGMENTS + 1) * 2, indices, MAX_SEGMENTS * 6);
    }

    // Helper to easily render text centered or aligned
    void RenderText(SDL_Renderer* ren, TTF_Font* font, const std::string& text, float x, float y, SDL_Color color, bool center_x = false, int font_size = 24) {
        if (!font || text.empty()) return;
        
        // SDL3 TTF text properties scale adjustment
        TTF_SetFontSize(font, font_size);
        SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), text.length(), color);
        if (!surface) return;

        SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
        if (!texture) {
            SDL_DestroySurface(surface);
            return;
        }

        float w = (float)surface->w;
        float h = (float)surface->h;
        float target_x = center_x ? x - (w / 2.0f) : x;

        SDL_FRect dst_rect = { target_x, y, w, h };
        SDL_RenderTexture(ren, texture, nullptr, &dst_rect);

        SDL_DestroyTexture(texture);
        SDL_DestroySurface(surface);
    }

    void update(const fh6_data & data_out) {
        // Clear background with transparency
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        // UI positioning dynamically based on current window layout
        float cx = WIDTH / 2.0f;
        float cy = HEIGHT / 2.0f;
        float radius = 90.0f;
        float thickness = 4.0f;

        // 1. Calculate dynamic RPM percentage mapping
        float rpm_pct = 0.0f;
        if (data_out.EngineMaxRpm > data_out.EngineIdleRpm) {
            rpm_pct = (data_out.CurrentEngineRpm - data_out.EngineIdleRpm) / (data_out.EngineMaxRpm - data_out.EngineIdleRpm);
            if (rpm_pct < 0.0f) rpm_pct = 0.0f;
            if (rpm_pct > 1.0f) rpm_pct = 1.0f;
        }

        // Arc starts at bottom-right (45 deg) and wraps counter-clockwise to top-right (-45 or 315 deg)
        float start_angle = 95.0f;
        float total_sweep = 200.0f; 
        float current_end_angle = start_angle + (rpm_pct * total_sweep);

        // 2. Color setups matching original UI
        SDL_Color white = { 255, 255, 255, 255 };
        SDL_Color gray  = { 100, 100, 100, 150 };
        SDL_Color red   = { 230, 40, 40, 255 };
        // SDL_Color blue  = { 50, 180, 245, 255 };

        // 3. Draw RPM Background Tracks & Active Progress
        // Draw the full background track in muted grey
        DrawArc(renderer, cx, cy, radius, start_angle, start_angle + total_sweep, thickness, gray);

        // Redline arc zone (top 15% of the powerband)
        float redline_pct = 0.85f;
        float redline_start = start_angle + (total_sweep * redline_pct);
        DrawArc(renderer, cx, cy, radius + 1.0f, redline_start, start_angle + total_sweep, thickness + 1.0f, red);

        // Dynamic filled arc up to the current RPM position (stops if it hits redline)
        float current_white_end = std::min(current_end_angle, redline_start);
        DrawArc(renderer, cx, cy, radius, start_angle, current_white_end, thickness, white);

        // If RPM is in the redline range, overlay the white progress bar with red
        if (current_end_angle > redline_start) {
            DrawArc(renderer, cx, cy, radius + 1.0f, redline_start, current_end_angle, thickness + 1.0f, red);
        }

        // 4. Draw RPM Numeric Tick Labels (0, 2, 4, 6 scaled by Max RPM)
        int max_val = static_cast<int>(std::round(data_out.EngineMaxRpm / 1000.0f));
        int steps[] = { 0, max_val / 3, (max_val * 2) / 3, max_val };

        for (int i = 0; i < 4; ++i) {
            float t = (float)i / 3.0f;
            float angle = start_angle + (t * total_sweep);
            float rad = angle * (M_PI / 180.0f);
            
            // Push text offset slightly outward from the line
            float tx = cx + (radius + 18.0f) * std::cos(rad);
            float ty = cy + (radius + 18.0f) * std::sin(rad);

            RenderText(renderer, font, std::to_string(steps[i]), tx, ty - 8, gray, true, 14);
        }

        // 5. Draw Gear Indicator & Underline
        std::string gear_str = (data_out.Gear == 0) ? "N" : ((data_out.Gear == 11) ? "N" : std::to_string(data_out.Gear));
        RenderText(renderer, font, gear_str, cx, cy - 45, white, true, 76);

        // // Underline beneath the Gear value
        // SDL_FRect line_rect = { cx - 25.0f, cy + 25.0f, 50.0f, 3.0f };
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // SDL_RenderFillRect(renderer, &line_rect);

        // // 6. Draw Throttle/Brake Vertical Gauges (Simulated/Placeholder visualization via basic indicators)
        // SDL_FRect brake_bar = { cx + 35.0f, cy - 20.0f, 3.0f, 40.0f };
        // SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        // SDL_RenderFillRect(renderer, &brake_bar);

        // SDL_FRect throttle_bar = { cx + 43.0f, cy - 20.0f, 3.0f, 40.0f };
        // SDL_SetRenderDrawColor(renderer, blue.r, blue.g, blue.b, blue.a);
        // SDL_RenderFillRect(renderer, &throttle_bar);

        // 7. Render Speedometer reading (VelocityZ converted from m/s to KPH)
        int speed_mph = static_cast<int>(std::abs(data_out.VelocityZ) * 3.6f);
        RenderText(renderer, font, std::to_string(speed_mph), cx + 55.0f, cy + 50.0f, white, true, 34);
        RenderText(renderer, font, "KPH", cx + 55.0f, cy + 85.0f, gray, true, 14);

        SDL_RenderPresent(renderer);
    }

    void close() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
    }
}

