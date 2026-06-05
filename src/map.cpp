#include <cstdio>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <algorithm>

#include "../include/map.hpp"
#include "../include/date.hpp"

namespace map{

    static constexpr unsigned short WIDTH = 780;
    static constexpr unsigned short HEIGHT = 996;

    // Manual measurements of coordinates:
    //  3048  8063 -> 577  81 =  157 -418 =>  157  418 = 0,051509 0,051842
    // -7350 -3152 ->  37 662 = -383  163 => -383 -163 = 0,052109 0,051713

    static constexpr unsigned short MAX_ORIGIN_X = 420;
    static constexpr unsigned short MAX_ORIGIN_Y = 499;

    static constexpr float SCALE_FACTOR = (0.051509 + 0.051842 + 0.052109 + 0.051713) / 4.f;

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

    static void draw_point(const SDL_Point& point) {
        SDL_SetRenderDrawColor(renderer, 255, 127, 0, 255);
        const SDL_FRect rect {point.x -3.f , point.y - 3.f,7,7};
        SDL_RenderFillRect(renderer, &rect);
    }

    static void map_texture() {
        static SDL_Texture* static_map_tex = nullptr;
        if (!static_map_tex) {
            const date first_season_start {21,5,2026};
            // const date today = get_today();
            const date today = {12,6,2026};

            unsigned int weeks_since_start = (date_to_int(today) - date_to_int(first_season_start)) / 7;

            SDL_Surface* surf;
            switch (weeks_since_start % 4) {
                case 0:
                    surf = SDL_LoadPNG("assets/maps/summer.png");
                    break;
                case 1:
                    surf = SDL_LoadPNG("assets/maps/autumn.png");
                    break;
                case 2:
                    surf = SDL_LoadPNG("assets/maps/winter.png");
                    break;
                case 3:
                    surf = SDL_LoadPNG("assets/maps/spring.png");
                    break;
            }
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

        // reduce from 3 to 2 dimension
        float pos_x = data_out.PositionX;
        float pos_y = data_out.PositionZ;

        SDL_Point position {static_cast<int>(pos_x * SCALE_FACTOR) + MAX_ORIGIN_X, static_cast<int>(pos_y * (-SCALE_FACTOR)) + MAX_ORIGIN_Y};
        static SDL_Point last_position = position;

        map_texture();

        if(!(pos_x == 0.f and pos_x == 0.f)) {
            // Stops jumping to (0,0) when paused.
            last_position = position;
        }
        draw_point(last_position);
        SDL_RenderPresent(renderer);
    }

    void close() {

    }

}