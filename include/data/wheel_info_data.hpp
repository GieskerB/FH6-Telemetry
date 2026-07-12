#ifndef WHEEL_INFO_DATA_HPP
#define WHEEL_INFO_DATA_HPP

#include <SDL3/SDL_pixels.h>

struct wheel_info_data {
    bool is_paused = false;
    unsigned short new_data = 0;
    SDL_Color slip_fl = {0, 0, 0, 0};
    SDL_Color slip_fr = {0, 0, 0, 0};
    SDL_Color slip_rl = {0, 0, 0, 0};
    SDL_Color slip_rr = {0, 0, 0, 0};
    char ground_fl[30]{0};
    char ground_fr[30]{0};
    char ground_rl[30]{0};
    char ground_rr[30]{0};
    char wheel_speed_fl[4]{0};
    char wheel_speed_fr[4]{0};
    char wheel_speed_rl[4]{0};
    char wheel_speed_rr[4]{0};
    float suspension_fl = 0;
    float suspension_fr = 0;
    float suspension_rl = 0;
    float suspension_rr = 0;
};

#endif
