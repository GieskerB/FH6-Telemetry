#ifndef ENGINE_RPM_DATA
#define ENGINE_RPM_DATA

#include <SDL3/SDL_pixels.h>

constexpr const char static_kmh_text[] = "KM/H";

struct engine_rpm_data {
    bool new_data = false;
    char gear[3]{0};
    char speed[4]{0};
    SDL_Color rpm_bar_color{0,0,0,0};
};

#endif
