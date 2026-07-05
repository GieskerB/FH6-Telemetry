#ifndef CAR_INFO_DATA_HPP
#define CAR_INFO_DATA_HPP

#include <SDL3/SDL_pixels.h>

constexpr unsigned char TEXT_WIDTH = 25;

struct car_info_data {
    bool is_paused = false;
    unsigned char new_data = 0;
    char drivetrain_path[23]{0};
    char class_id[3]{0};
    SDL_Color class_color{0,0,0,0};
    char performance_id[4]{0};
    char flag_path[29]{0};
    char group[TEXT_WIDTH]{0};
    char year_make[TEXT_WIDTH]{0};
    char model[TEXT_WIDTH]{0};
};

#endif
