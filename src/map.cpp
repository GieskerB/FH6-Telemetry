#include <cstdio>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>

#include "../include/map.hpp"
#include "../include/util/date.hpp"
#include "../include/util/colors.hpp"
#include "../include/util/texture_handler.hpp"

namespace map{

    static constexpr unsigned short WIDTH = 780;
    static constexpr unsigned short HEIGHT = 996;

    // Manual measurements of coordinates:
    //  3048  8063 -> 577  81 =  157 -418 =>  157  418 = 0,051509 0,051842
    // -7350 -3152 ->  37 662 = -383  163 => -383 -163 = 0,052109 0,051713

    static constexpr unsigned short MAX_ORIGIN_X = 420;
    static constexpr unsigned short MAX_ORIGIN_Y = 499;

    static constexpr unsigned char ARROW_SIZE = 17;

    static constexpr double PI = 3.14159265358979323846;

    static constexpr float SCALE_FACTOR = (0.051509 + 0.051842 + 0.052109 + 0.051713) / 4.f;

    static const char* SEASONS[] = {"assets/maps/summer.png","assets/maps/autumn.png","assets/maps/winter.png","assets/maps/spring.png"};

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

    static void draw_nav_arrow(float pos_x, float pos_z, float yaw) {

        // reduce from 3 to 2 dimension
        const SDL_Point position {static_cast<int>(pos_x * SCALE_FACTOR) + MAX_ORIGIN_X, static_cast<int>(pos_z * (-SCALE_FACTOR)) + MAX_ORIGIN_Y};
        
        //Convert from rad to deg:
        yaw = yaw * 180 / PI;
        const int rotation = (std::round((yaw) / 5)) * 5;
        static int last_rotation = -1;

        static SDL_Texture* arrow_tex = nullptr;
        if (!arrow_tex or last_rotation != rotation) {
            if (arrow_tex) SDL_DestroyTexture(arrow_tex);
            
            char buffer[26]{0};
            SDL_snprintf(buffer, sizeof(buffer), "assets/arrows/nav-%03d.png", rotate_correctly(rotation));

            SDL_Surface* surf  = SDL_LoadPNG(buffer);
            if (surf) {
                arrow_tex = SDL_CreateTextureFromSurface(renderer, surf);
                last_rotation = rotation;
                SDL_DestroySurface(surf);
            }
        }
        if (arrow_tex) {
            float texture_size = 0;
            // should be between 21 and 30!
            SDL_GetTextureSize(arrow_tex,&texture_size,&texture_size);
            const float scalar = texture_size / 30;
            const SDL_FRect rect = {
                position.x - (ARROW_SIZE * scalar / 2 +1.f),
                position.y - ARROW_SIZE * scalar/ 2 +1.f,
                ARROW_SIZE * scalar,
                ARROW_SIZE * scalar
            };
            SDL_RenderTexture(renderer, arrow_tex, nullptr, &rect);
        }
    }

    static void draw_map() {
        static SDL_Texture* map_tex = nullptr;
        if (!map_tex) {
            const date first_season_start {21,5,2026};
            const date today = get_today();
            const unsigned int weeks_since_start = (date_to_int(today) - date_to_int(first_season_start)) / 7;

            texture_png_static(renderer,&map_tex,SEASONS[weeks_since_start % 4]);
        }
        if (map_tex) {
            static const SDL_FRect rect = {0,0,WIDTH,HEIGHT };
            SDL_RenderTexture(renderer, map_tex, nullptr, &rect);
        }
    }

    void update(const fh6_data& data_out) {
        if(data_out.PositionX == 0.f and data_out.PositionZ == 0.f) {
            // In menu!
            return;
        }

        SDL_SetRenderDrawColor(renderer, 15, 15, 20, 255);
        SDL_RenderClear(renderer);

        draw_map();
        draw_nav_arrow(data_out.PositionX, data_out.PositionZ, data_out.Yaw);
        
        SDL_RenderPresent(renderer);
    }

    void close() {

    }

}
