#include "../include/map.hpp"

#include <SDL3/SDL.h>
#include <stdlib.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "../include/util/colors.hpp"
#include "../include/util/date.hpp"
#include "../include/util/texture_handler.hpp"

static unsigned short WIDTH;
static unsigned short HEIGHT;

// Manual measurements of coordinates:
//  3048  8063 -> 577  81 =  157 -418 =>  157  418 = 0,051509 0,051842
// -7350 -3152 ->  37 662 = -383  163 => -383 -163 = 0,052109 0,051713

static unsigned short MAP_ORIGIN_X;
static unsigned short MAP_ORIGIN_Y;

static float SCALE_FACTOR;

static unsigned char ARROW_SIZE;

static constexpr double PI = 3.14159265358979323846;

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

void map_t::init(unsigned short size) {
    WIDTH = size;
    HEIGHT = static_cast<unsigned short>(996.f / 780.f * size);

    MAP_ORIGIN_X = static_cast<unsigned short>(420.f / 780.f * WIDTH);
    MAP_ORIGIN_Y = static_cast<unsigned short>(499.f / 996.f * HEIGHT);

    SCALE_FACTOR = (0.051509 + 0.051842 + 0.052109 + 0.051713) / 4.f * (size / 780.f);

    ARROW_SIZE = 17.f / 780.f * size;

    window = SDL_CreateWindow("Map", WIDTH, HEIGHT, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
    if (window == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

static const std::string& update_map_path(const date& today, unsigned char& changed) {
    static const char* SEASONS[] = {"assets/maps/summer.png", "assets/maps/autumn.png", "assets/maps/winter.png",
                                    "assets/maps/spring.png"};
    static const date first_season_start{21, 5, 2026};
    static unsigned int last_date_id = date_to_int(first_season_start);
    static std::string return_value{};
    const unsigned int date_id = date_to_int(today);
    if (date_id != last_date_id) {
        last_date_id = date_id;
        const unsigned int weeks_since_start = (date_id - last_date_id) / 7;
        return_value = SEASONS[weeks_since_start % 4];
        changed |= 0b1;
    }
    return return_value;
}
static const std::string& update_arrow_path(float yaw, unsigned char& changed) {
    float rotation = (std::round((yaw * 180 / PI) / 5)) * 5;
    if (rotation < 0) {
        rotation += 360;
    }
    static float last_rotation = -1;
    static std::string return_value{};
    if (rotation != last_rotation) {
        last_rotation = rotation;
        std::stringstream strstream;
        strstream << "assets/arrows/nav-" << std::setw(3) << std::setfill('0') << rotation << ".png";
        return_value = strstream.str();
        changed |= 0b10;
    }
    return return_value;
}
static const SDL_Point& update_arrow_position(float pos_x, float pos_z, unsigned char& changed) {
    static float last_pos_x = -1;
    static float last_pos_z = -1;
    static SDL_Point return_value;
    if (pos_x != last_pos_x or pos_z != last_pos_z) {
        last_pos_x = pos_x;
        last_pos_z = pos_z;
        return_value.x = static_cast<int>(pos_x * SCALE_FACTOR) + MAP_ORIGIN_X;
        return_value.y = static_cast<int>(pos_z * (-SCALE_FACTOR)) + MAP_ORIGIN_Y;
        changed |= 0b10000000;
    }
    return return_value;
}

void map_t::update(const fh6_data& data_out) {
    const bool is_paused = data_out.PositionX == 0 and data_out.PositionY == 0 and data_out.PositionZ == 0;

    if (is_paused) {
        mutex->lock();
        data.is_paused = is_paused;
        mutex->unlock();
        return;
    }

    unsigned char changes = 0;
    const std::string& map_path = update_map_path(get_today(), changes);
    const std::string& arrow_path = update_arrow_path(data_out.Yaw, changes);
    const SDL_Point& arrow_position = update_arrow_position(data_out.PositionX, data_out.PositionZ, changes);

    if (changes != 0) {
        mutex->lock();
        data.is_paused = is_paused;
        data.new_data = changes;
        std::strncpy(data.season_map_path, map_path.c_str(), sizeof(data.season_map_path) - 1);
        std::strncpy(data.arrow_path, arrow_path.c_str(), sizeof(data.arrow_path) - 1);
        data.arrow_position = arrow_position;
        mutex->unlock();
    }
}

static void render_season_map(const char* map_path, bool changed) {
    static SDL_Texture* texture = nullptr;
    if (changed or !texture) texture_png(renderer, &texture, map_path);
    if (texture) {
        static const SDL_FRect rect = {0, 0, static_cast<float>(WIDTH), static_cast<float>(HEIGHT)};
        SDL_RenderTexture(renderer, texture, nullptr, &rect);
    }
}
static void render_arrow(const char* arrow_path, const SDL_Point& arrow_position, bool changed) {
    static SDL_Texture* texture = nullptr;
    if (changed or !texture) texture_png(renderer, &texture, arrow_path);
    if (texture) {
        float texture_size = 0;
        // should be between 21 and 30!
        SDL_GetTextureSize(texture, &texture_size, &texture_size);
        const float scalar = texture_size / 30;
        const float offset = ARROW_SIZE * scalar / 2 + 1;
        const float size = ARROW_SIZE * scalar;
        const SDL_FRect rect = {arrow_position.x - offset, arrow_position.y - offset, size, size};
        SDL_RenderTexture(renderer, texture, nullptr, &rect);
    }
}

void map_t::render() {
    map_data data_copy;
    mutex->lock();
    if (data.new_data == 0 or data.is_paused) {
        mutex->unlock();
        return;
    }
    std::memcpy(&data_copy, &data, sizeof(data));
    mutex->unlock();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    render_season_map(data_copy.season_map_path, (data_copy.new_data & 0b1) == 0b1);
    render_arrow(data_copy.arrow_path, data_copy.arrow_position, (data_copy.new_data & 0b10) == 0b10);

    SDL_RenderPresent(renderer);
}

void map_t::close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
