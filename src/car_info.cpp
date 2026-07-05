#include <cstdio>
#include <stdlib.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <string>
#include <format>
#include <iostream>
#include <sstream>
#include <cstring>

#include "../include/car_info.hpp"
#include "../include/util/colors.hpp"
#include "../include/util/texture_handler.hpp"
#include "../include/util/csv_to_maps.hpp"

static constexpr float SPRITE_RATIO = 197.f / 125.f;
static constexpr float SPRITE_PORTION = 0.2f;
static unsigned short WIDTH, HEIGHT;
static unsigned short SPRITE_WIDTH, SPRITE_HEIGHT;
static unsigned short PADDING;

static SDL_Window * window = nullptr;
static SDL_Renderer* renderer = nullptr;
static TTF_Font* pi_font = nullptr;
static TTF_Font* text_font = nullptr;

void car_info_t::init(unsigned short size) {

    WIDTH = size;
    SPRITE_WIDTH = WIDTH * SPRITE_PORTION;
    SPRITE_HEIGHT = SPRITE_WIDTH * SPRITE_RATIO;
    HEIGHT = SPRITE_HEIGHT + SPRITE_WIDTH;
    PADDING = HEIGHT * 0.05;

    window = SDL_CreateWindow("Car Info",WIDTH ,HEIGHT,SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
    if(window == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    renderer = SDL_CreateRenderer(window, NULL);
    if(renderer == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    pi_font = TTF_OpenFont("assets/fonts/digital-7.ttf",150);
    if(pi_font == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    text_font = TTF_OpenFont("assets/fonts/droid-sans.ttf",64);
    if(text_font == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    mutex = std::make_unique<std::mutex>();
}

static std::string update_drivetrain_path(int drivetrain, unsigned char& changed) {
    static const std::string PATHS[] = {"assets/sprites/FWD.png","assets/sprites/RWD.png","assets/sprites/AWD.png"};
    static int last_drivetrain_type = -1;
    static std::string return_value{};
    if (drivetrain != last_drivetrain_type) {
        last_drivetrain_type = drivetrain;
        return_value = PATHS[drivetrain];
        changed |= 0b1;
    }
    return return_value;
}
static std::string update_class_id(int class_id, unsigned char& changed) {
    static const std::string CLASSES[] = {" D", " C", " B", " A", "S1", "S2", " R", " X"};
    static int last_class_id = -1;
    static std::string return_value{};
    if (class_id != last_class_id) {
        last_class_id =class_id;
        return_value = CLASSES[class_id];
        changed |= 0b10;
    }
    return return_value;
}
static SDL_Color update_class_color(int class_id, unsigned char& changed) {
    static constexpr SDL_Color CLASS_COLORS[8] = {{103, 185, 238, 255}, {246, 198, 85,255},
                                                {236, 109, 65, 255}, {233, 61, 78, 255},
                                                {172, 100, 224, 255}, {49, 93, 210, 255},
                                                {195, 53, 151, 255}, {101, 212, 104, 255}};
    static int last_class_id = -1;
    static SDL_Color return_value{0,0,0,0};
    if (class_id != last_class_id) {
        last_class_id =class_id;
        return_value = CLASS_COLORS[class_id];
        changed |= 0b100;
    }
    return return_value;
}
static std::string update_performance_id(int performance_id, unsigned char& changed) {
    static int last_performance_id = -1;
    static std::string return_value{};
    if (performance_id != last_performance_id) {
        last_performance_id = performance_id;
        return_value = std::to_string(performance_id);
        changed |= 0b1000;
    }
    return return_value;
}
static std::string update_flag_path(const std::string& country, unsigned char& changed) {
    static std::string last_country;
    static std::string return_value{};
    if (country != last_country) {
        last_country = country;
        std::stringstream strstream;
        strstream << "assets/flags/" << country << ".png";
        return_value = strstream.str();
        changed |= 0b10000;
    }
    return return_value;
}
static std::string update_group(const std::string& group, unsigned char& changed) {
    static std::string last_group;
    // static std::string return_value{};
    if (group!= last_group) {
        last_group = group;
        // return_value = group;
        changed |= 0b100000;
    }
    return last_group; //return_value;
}
static std::string update_year_make(int year, const std::string& make, unsigned char& changed) {
    static auto car_map = car_details_map();
    static int last_year = -1;
    static std::string last_make;
    static std::string return_value{};
    if (year != last_year or make != last_make) {
        last_year = year;
        last_make = make;
        std::stringstream strstream;
        strstream << year << " - " << make;
        return_value = strstream.str();
        changed |= 0b1000000;
    }
    return return_value;
}
static std::string update_model(const std::string& model, unsigned char& changed) {
    static std::string last_model;
    // static std::string return_value{};
    if (model != last_model) {
        last_model = model;
        // return_value = model;
        changed |= 0b10000000;
    }
    return last_model; //return_value;
}

void car_info_t::update(const fh6_data& data_out) {
    static auto details_map = car_details_map();
    static auto group_map = car_group_map();
    static auto country_map = car_country_map();

    const bool is_paused = data_out.PositionX == 0 and data_out.PositionY == 0 and data_out.PositionZ == 0;

    if(is_paused) {
        mutex->lock();
        data.is_paused = is_paused;
        mutex->unlock();
        return;
    }

    const auto details = details_map[data_out.CarOrdinal];

    unsigned char changes = 0;
    std::string drivetrain_path = update_drivetrain_path(data_out.DrivetrainType,changes);
    std::string class_id = update_class_id(data_out.CarClass,changes);
    SDL_Color class_color = update_class_color(data_out.CarClass,changes);
    std::string performance_id = update_performance_id(data_out.CarPerformanceIndex,changes);
    std::string flag_path = update_flag_path(country_map[details.make],changes);
    std::string group = update_group(group_map[data_out.CarGroup],changes);
    std::string year_make = update_year_make(details.year, details.make,changes);
    std::string model = update_model(details.model,changes);

    if (changes != 0) {
        mutex->lock();
        data.is_paused = is_paused;
        data.new_data = changes;
        std::strncpy(data.drivetrain_path,drivetrain_path.c_str(), sizeof(data.drivetrain_path)-1);
        std::strncpy(data.class_id,class_id.c_str(), sizeof(data.class_id)-1);
        data.class_color = class_color;
        std::strncpy(data.performance_id,performance_id.c_str(), sizeof(data.performance_id)-1);
        std::strncpy(data.flag_path,flag_path.c_str(), sizeof(data.flag_path)-1);
        std::strncpy(data.group,group.c_str(), sizeof(data.group)-1);
        std::strncpy(data.year_make,year_make.c_str(), sizeof(data.year_make)-1);
        std::strncpy(data.model,model.c_str(), sizeof(data.model)-1);
        mutex->unlock();
    }
}

static void render_drivetrain(const char* path) {
    static SDL_Texture* texture = nullptr;
    texture_png(renderer,&texture,path);
    if (texture) {
        static const SDL_FRect unit_rect = {static_cast<float>(WIDTH - SPRITE_WIDTH), 0, static_cast<float>(SPRITE_WIDTH), static_cast<float>(SPRITE_HEIGHT)};
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_class_id(const char* value, const SDL_Color& color) {
    static SDL_Texture* texture = nullptr;
    texture_text(renderer, &texture, value, pi_font, WHITE);
    if (texture) {
        // Let ID take up 40 % of the space on the left
        static const float ID_SPACE = 0.4f;
        static const SDL_FRect class_bg = { 0, 0, static_cast<float>(WIDTH - SPRITE_WIDTH) * ID_SPACE, static_cast<float>(SPRITE_HEIGHT)};
        static const SDL_FRect rest_bg = {static_cast<float>(WIDTH - SPRITE_WIDTH) * ID_SPACE, 0, static_cast<float>(WIDTH - SPRITE_WIDTH) * (1-ID_SPACE),static_cast<float>(SPRITE_HEIGHT)};

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &class_bg);
        SDL_RenderFillRect(renderer, &rest_bg);

        const SDL_FRect text_rect = {
            class_bg.x + class_bg.w * 0.125f - ((value[0]== ' ') ? class_bg.w * 0.75f / 4 : 0),
            class_bg.y + class_bg.h * 0.125f,
            class_bg.w * 0.75f,
            class_bg.h * 0.75f
        };

        SDL_RenderTexture(renderer, texture, nullptr, &text_rect);
    }
}
static void render_performance_id(const char* value) {
    static SDL_Texture* texture = nullptr;
    texture_text(renderer, &texture, value, pi_font, WHITE);
    if (texture) {
        // Let PI take up 60 % of the space on the right - some small padding
        static const SDL_FRect performance_bg = {
            WIDTH * (1-SPRITE_PORTION) * 0.4f + PADDING,
            static_cast<float>(PADDING),
            WIDTH * (1-SPRITE_PORTION) * 0.6f -2* PADDING,
            static_cast<float>(SPRITE_HEIGHT - 2 * PADDING)};

        SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, 255);
        SDL_RenderFillRect(renderer, &performance_bg);

        // Center using the newly scaled dimensions
        static const SDL_FRect text_rect = {
            performance_bg.x + performance_bg.w * 0.125f,
            performance_bg.y + performance_bg.h * 0.125f,
            performance_bg.w * 0.75f,
            performance_bg.h * 0.75f
        };

        SDL_RenderTexture(renderer, texture, nullptr, &text_rect);
    }
}
static void render_flag(const char* path) {
    static SDL_Texture* texture = nullptr;
    texture_png(renderer,&texture,path);
    if (texture) {
        static const SDL_FRect unit_rect = {WIDTH * (1 - SPRITE_PORTION), static_cast<float>(SPRITE_HEIGHT), static_cast<float>(SPRITE_WIDTH), static_cast<float>(SPRITE_WIDTH)};
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_group(const char* value) {
    static SDL_Texture* texture = nullptr;
    texture_text(renderer, &texture, value, text_font, WHITE);
    if (texture) {
        const SDL_FRect unit_rect = { 0, static_cast<float>(SPRITE_HEIGHT), static_cast<float>(WIDTH-SPRITE_WIDTH), static_cast<float>(SPRITE_WIDTH / 3)};
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_year_make(const char* value) {
    static SDL_Texture* texture = nullptr;
    texture_text(renderer, &texture, value, text_font, WHITE);
    if (texture) {
        const SDL_FRect unit_rect = { 0, static_cast<float>(SPRITE_HEIGHT + SPRITE_WIDTH / 3), static_cast<float>(WIDTH-SPRITE_WIDTH), static_cast<float>(SPRITE_WIDTH / 3)};
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_model(const char* value) {
    static SDL_Texture* texture = nullptr;
    texture_text(renderer, &texture, value, text_font, WHITE);
    if (texture) {
        const SDL_FRect unit_rect = { 0, static_cast<float>(SPRITE_HEIGHT + SPRITE_WIDTH *2 / 3), static_cast<float>(WIDTH-SPRITE_WIDTH), static_cast<float>(SPRITE_WIDTH / 3)};
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}

void car_info_t::render() {
    car_info_data data_copy;
    mutex->lock();
    if (data.new_data == 0 or data.is_paused) {
        mutex->unlock();
        return;
    }
    std::memcpy(&data_copy,&data,sizeof(data));
    mutex->unlock();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    if (data_copy.new_data & 0b1) render_drivetrain(data_copy.drivetrain_path);
    if (data_copy.new_data & 0b10) render_class_id(data_copy.class_id,data_copy.class_color);
    if (data_copy.new_data & 0b1000) render_performance_id(data_copy.performance_id);
    if (data_copy.new_data & 0b10000) render_flag(data_copy.flag_path);
    if (data_copy.new_data & 0b100000) render_group(data_copy.group);
    if (data_copy.new_data & 0b1000000) render_year_make(data_copy.year_make);
    if (data_copy.new_data & 0b10000000) render_model(data_copy.model);

    SDL_RenderPresent(renderer);
}

void car_info_t::close() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (pi_font) TTF_CloseFont(pi_font);
    if (text_font) TTF_CloseFont(text_font);
}
