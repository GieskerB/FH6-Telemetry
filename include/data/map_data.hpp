#ifndef MAP_DATA
#define MAP_DATA

#include <SDL3/SDL_rect.h>

constexpr const char static_background_path[] = "assets/sprites/gforce_background.png";
constexpr const char gforce_label[][3] = {{"1G"},{"2G"},{"3G"},{"4G"}};
constexpr const char speed_label[][4] = {{"100"},{"200"},{"300"},{"400"}};

struct car_info_data {
    bool new_data = false;
    char season_map_path[23]{0};
    char arrow_path[26]{0};
    SDL_Point arrow_position{0,0};
};

#endif
