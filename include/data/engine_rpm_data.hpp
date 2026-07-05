#ifndef ENGINE_RPM_DATA
#define ENGINE_RPM_DATA

#include <SDL3/SDL_pixels.h>

constexpr const char static_kmh_text[] = "KM/H";

struct engine_rpm_data {
    bool is_paused = false;
    unsigned char new_data = 0;
    char gear[3]{0};
    char speed[4]{0};
    int idle_rpm = 0;
    int current_rpm = 0;
    int max_rpm = 0;
    SDL_Color rpm_bar_color{0,0,0,0};
};

#endif
