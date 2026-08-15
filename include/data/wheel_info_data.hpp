#ifndef WHEEL_INFO_DATA_HPP
#define WHEEL_INFO_DATA_HPP

#include <SDL3/SDL_pixels.h>

const char static_speed_text[][8] {"FL-Tire","FR-Tire","RL-Tire","RR-Tire"};

const char static_tire_path[] = "assets/sprites/Tire.png";
const char static_lateral_path[] = "assets/sprites/Lateral.png";
const char static_suspension_path[] = "assets/sprites/Suspension.png";

struct wheel_info_data {
    bool is_paused = false;
    unsigned int new_data = 0;
    SDL_Color slipping[4] {0, 0, 0, 0};
    char temperature[4][9]{0}; // one extra byte because of ° symbol!
    char wheel_speed[4][8]{0};
    float lateral_slip[4]{0};
    float suspension[4]{0};
};

#endif
