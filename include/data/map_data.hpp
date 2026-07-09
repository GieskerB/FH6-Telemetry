#ifndef MAP_DATA
#define MAP_DATA

#include <SDL3/SDL_rect.h>

struct map_data {
    bool is_paused = false;
    unsigned char new_data = 0;
    char season_map_path[23]{0};
    char arrow_path[26]{0};
    SDL_Point arrow_position{0,0};
};

#endif
