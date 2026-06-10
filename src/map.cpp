#include <cstdio>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <math.h>

#include "../include/map.hpp"
#include "../include/date.hpp"
#include "../include/colors.hpp"

namespace map{

    static constexpr unsigned short WIDTH = 780;
    static constexpr unsigned short HEIGHT = 996;

    // Manual measurements of coordinates:
    //  3048  8063 -> 577  81 =  157 -418 =>  157  418 = 0,051509 0,051842
    // -7350 -3152 ->  37 662 = -383  163 => -383 -163 = 0,052109 0,051713

    static constexpr unsigned short MAX_ORIGIN_X = 420;
    static constexpr unsigned short MAX_ORIGIN_Y = 499;

    static constexpr unsigned char ARROW_SIZE = 13;

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

    static int rotate_correctly(int rotation) {
        if (rotation < 0) {
            return 360 + rotation;
        }
        return rotation;
    }

    static void draw_nav_arrow(const SDL_Point& point, float rotation) {
        const int rotation_step = (std::round(rotation / 5)) * 5;
        static SDL_Texture* arrow_tex = nullptr;
        static int last_rotation = rotation_step;
        if (!arrow_tex or last_rotation != rotation_step) {
            if (arrow_tex) SDL_DestroyTexture(arrow_tex);
            
            char buffer[26]{0};
            SDL_snprintf(buffer, sizeof(buffer), "assets/arrows/nav-%03d.png", rotate_correctly(rotation_step));

            SDL_Surface* surf  = SDL_LoadPNG(buffer);
            if (surf) {
                arrow_tex = SDL_CreateTextureFromSurface(renderer, surf);
                last_rotation = rotation_step;
                SDL_DestroySurface(surf);
            }
        }
        if (arrow_tex) {
            const SDL_FRect rect = {point.x - (ARROW_SIZE / 2 +1.f) , point.y - ARROW_SIZE / 2 +1.f, ARROW_SIZE, ARROW_SIZE};
            SDL_RenderTexture(renderer, arrow_tex, nullptr, &rect);
        }
    }

    static void draw_map() {
        static SDL_Texture* map_tex = nullptr;
        if (!map_tex) {
            const date first_season_start {21,5,2026};
            const date today = get_today();

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
                map_tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_DestroySurface(surf);
            }
        }
        if (map_tex) {
            static const SDL_FRect rect = {0,0,WIDTH,HEIGHT };
            SDL_RenderTexture(renderer, map_tex, nullptr, &rect);
        }
    }

    void update(const fh6_data& data_out) {
        SDL_SetRenderDrawColor(renderer, 15, 15, 20, 255);
        SDL_RenderClear(renderer);

        draw_map();

        // reduce from 3 to 2 dimension
        const float pos_x = data_out.PositionX;
        const float pos_y = data_out.PositionZ;
        SDL_Point position {static_cast<int>(pos_x * SCALE_FACTOR) + MAX_ORIGIN_X, static_cast<int>(pos_y * (-SCALE_FACTOR)) + MAX_ORIGIN_Y};
        static SDL_Point last_position = position;
        
        const float rotation = data_out.Yaw * 180 / M_PI;
        static float last_rotation = rotation;

        // Stops jumping to (0,0) when paused.
        if(!(pos_x == 0.f and pos_x == 0.f)) {
            last_position = position;
            last_rotation = rotation;
        }

        draw_nav_arrow(last_position, last_rotation);
        SDL_RenderPresent(renderer);
    }

    void close() {

    }

}
