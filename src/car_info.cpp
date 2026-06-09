#include <cstdio>
#include <stdlib.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "../include/car_info.hpp"
#include <iostream>
#include <algorithm>

namespace car_info {

    static constexpr unsigned short SPRITE_WIDTH = 125 / 2;
    static constexpr unsigned short SPRITE_HEIGHT = 197 / 2;

    static constexpr unsigned short WIDTH = 200;
    static constexpr unsigned short HEIGHT = SPRITE_HEIGHT;

    static constexpr unsigned short PADDING = HEIGHT * 0.05;

    static constexpr SDL_Color WHITE = {250, 250, 250, 255};
    static constexpr SDL_Color BLACK = {5, 5, 5, 255};
    //Currently placeholder...
    static constexpr SDL_Color COLORS[8] = {{103, 185, 238, 255}, {246, 198, 85,255},
                                                  {236, 109, 65, 255}, {233, 61, 78, 255},
                                                  {172, 100, 224, 255}, {49, 93, 210, 255},
                                                  {195, 53, 151, 255}, {101, 212, 104, 255}};

    static SDL_Window * window = nullptr;
    static SDL_Renderer* renderer = nullptr;
    static TTF_Font* font = nullptr;

    void init() {
        window = SDL_CreateWindow("Car Info",WIDTH + SPRITE_WIDTH ,HEIGHT,SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
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

    static void drivetrain_texture(int drivetrain) {
        static SDL_Texture* cached_drivetrain_tex = nullptr;
        static int last_drivetrain = drivetrain;
        if (!cached_drivetrain_tex || last_drivetrain != drivetrain) {
            if (cached_drivetrain_tex) SDL_DestroyTexture(cached_drivetrain_tex);

            SDL_Surface* surf;
            switch (drivetrain)
            {
            case 0:
                surf = SDL_LoadPNG("assets/sprites/FWD.png");
                break;
            case 1:
                surf = SDL_LoadPNG("assets/sprites/RWD.png");
                break;
            case 2:
                surf = SDL_LoadPNG("assets/sprites/AWD.png");
                break;
            default:
                perror("Invalid drivetrain");
                exit(EXIT_FAILURE);
                break;
            }

            if (surf) {
                cached_drivetrain_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
                last_drivetrain = drivetrain;
            }
        }
        if (cached_drivetrain_tex) {
            static const SDL_FRect unit_rect = {WIDTH,0,SPRITE_WIDTH,HEIGHT}; // HALF IMAGE SIZE
            SDL_RenderTexture(renderer, cached_drivetrain_tex, nullptr, &unit_rect);
        }
    }

    static void class_id_texture(int id) {
        static SDL_Texture* cached_id_tex = nullptr;
        static int last_id = id;
        if (!cached_id_tex || last_id !=  id) {
            if (cached_id_tex) SDL_DestroyTexture(cached_id_tex);
            static const char* class_names[] = {"D", "C", "B", "A", "S1", "S2", "R", "X"};
            char buffer [3]{0};
            SDL_snprintf(buffer, sizeof(buffer), "%s", class_names[id]);

            SDL_Surface* surf = TTF_RenderText_Blended(font, buffer, 0, WHITE);
            if (surf) {
                cached_id_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
                last_id =id;
            }
        }
        if (cached_id_tex) {
            // Left side takes up 40% width, 100% height
            SDL_FRect class_bg = { 0, 0, WIDTH * 0.4f, HEIGHT};
            SDL_FRect rest_bg = {WIDTH * 0.4f,0,WIDTH * 0.6f + SPRITE_WIDTH,HEIGHT};
            
            SDL_SetRenderDrawColor(renderer, COLORS[id].r, COLORS[id].g, COLORS[id].b, COLORS[id].a);
            SDL_RenderFillRect(renderer, &class_bg);
            SDL_RenderFillRect(renderer, &rest_bg);

            // Get original text dimensions
            float tex_w = 0, tex_h = 0;
            SDL_GetTextureSize(cached_id_tex, &tex_w, &tex_h);

            // Calculate bounding box constraint (give it 25% padding inside the box)
            constexpr float max_w = WIDTH * 0.4f * 0.75f;
            constexpr float max_h = HEIGHT * 0.75f;
            
            // Proportional Scale Factor calculation
            const float scale = std::min(max_w / tex_w, max_h / tex_h);
            const float final_w = tex_w * scale;
            const float final_h = tex_h * scale;

            // Center using the newly scaled dimensions
            SDL_FRect text_rect = {
                (class_bg.w - final_w) / 2.0f,
                (class_bg.h - final_h) / 2.0f,
                final_w,
                final_h
            };
            
            SDL_RenderTexture(renderer, cached_id_tex, nullptr, &text_rect);
        }
    }

    static void class_ranking_texture(int performance) {
        static SDL_Texture* cached_performance_tex = nullptr;
        performance = std::clamp(performance,100,999);
        static int last_performance = performance;
        if (!cached_performance_tex || last_performance !=  performance) {
            if (cached_performance_tex) SDL_DestroyTexture(cached_performance_tex);
            char buffer [4]{0};
            SDL_snprintf(buffer, sizeof(buffer), "%d", performance);
            SDL_Surface* surf = TTF_RenderText_Blended(font, buffer, 0, WHITE);
            if (surf) {
                cached_performance_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
                last_performance = performance;
            }
        }
        if (cached_performance_tex) {
            // Right side takes up 60% width, 100% height
            SDL_FRect class_bg = {WIDTH * 0.4f + PADDING, PADDING,
                                 WIDTH * 0.6f -2* PADDING, HEIGHT - 2 * PADDING};
            
            SDL_SetRenderDrawColor(renderer, 15, 15, 15, 255); 
            SDL_RenderFillRect(renderer, &class_bg);

            // Get original text dimensions
            float tex_w = 0.0f, tex_h = 0.0f;
            SDL_GetTextureSize(cached_performance_tex, &tex_w, &tex_h);

            // Calculate bounding box constraint (give it 25% padding inside the box)
            float max_w = class_bg.w * 0.75f;
            float max_h = class_bg.h * 0.75f;
            
            // Proportional Scale Factor calculation
            float scale = std::min(max_w / tex_w, max_h / tex_h);
            float final_w = tex_w * scale;
            float final_h = tex_h * scale;

            // Center using the newly scaled dimensions
            SDL_FRect text_rect = {
                class_bg.x + (class_bg.w - final_w) / 2.f,
                class_bg.y + (class_bg.h - final_h) / 2.f,
                final_w,
                final_h
            };
            
            SDL_RenderTexture(renderer, cached_performance_tex, nullptr, &text_rect);
        }
    }

    void update(const fh6_data& data_out) {
        if (data_out.CarPerformanceIndex == 0) {
            //In menu!
            return;
        }
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 69, 69, 69, 69);
        SDL_RenderClear(renderer);
        
        class_id_texture(data_out.CarClass);
        class_ranking_texture(data_out.CarPerformanceIndex);
        drivetrain_texture(data_out.DrivetrainType);

        SDL_RenderPresent(renderer);
    }

    void close() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
    }

}