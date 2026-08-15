#ifndef MAP_DATA
#define MAP_DATA

#include <SDL3/SDL_rect.h>

constexpr const char static_arrow_path[] = "assets/sprites/Arrow.png";

struct map_data {
    bool is_paused = false;
    unsigned char new_data = 0;
    char season_map_path[23]{0};
    float arrow_angle = 0.f;
    SDL_Point arrow_position{0, 0};
};

#endif
