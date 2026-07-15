#ifndef GFORCE_DATA
#define GFORCE_DATA

#include <SDL3/SDL_rect.h>

constexpr const char static_background_path[] = "assets/sprites/GForce.png";
constexpr const char gforce_label[][3] = {{"1G"}, {"2G"}, {"3G"}, {"4G"}};
constexpr const char speed_label[][4] = {{"400"}, {"300"}, {"200"}, {"100"}};

struct gforce_data {
    bool is_paused = false;
    unsigned char new_data = 0;
    SDL_Point new_gforce_point{0, 0};
    SDL_Point new_speed_point{0, 0};
    char gforce[6]{0};
    char speed[8]{0};
};

#endif
