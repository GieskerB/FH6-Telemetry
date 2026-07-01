#include <cstdio>
#include <stdlib.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <string>
#include <format>
#include <iostream>

#include "../include/car_info.hpp"
#include "../include/util/colors.hpp"
#include "../include/util/texture_handler.hpp"
#include "../include/util/csv_to_maps.hpp"

static constexpr int TEXT_WIDTH = 25;
static constexpr float SPRITE_RATIO = 197.f / 125.f;
static constexpr float SPRITE_PORTION = 0.2f;
static unsigned short WIDTH, HEIGHT;
static unsigned short SPRITE_WIDTH, SPRITE_HEIGHT;
static unsigned short PADDING;

static constexpr SDL_Color CLASS_COLORS[8] = {{103, 185, 238, 255}, {246, 198, 85,255},
                                                {236, 109, 65, 255}, {233, 61, 78, 255},
                                                {172, 100, 224, 255}, {49, 93, 210, 255},
                                                {195, 53, 151, 255}, {101, 212, 104, 255}};

static const char* CLASS_IDS[] = {" D", " C", " B", " A", "S1", "S2", " R", " X"};
static const char* DRIVETRAIN_PNGS[] = {"assets/sprites/FWD.png","assets/sprites/RWD.png","assets/sprites/AWD.png"};

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
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    text_font = TTF_OpenFont("assets/fonts/droid-sans.ttf",256);
    if(text_font == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

static void draw_drivetrain(int drivetrain) {
    static SDL_Texture* drivetrain_texture = nullptr;
    static int last_drivetrain = -1;
    if (last_drivetrain != drivetrain) {
        texture_png(renderer,&drivetrain_texture,DRIVETRAIN_PNGS[drivetrain]);
        last_drivetrain = drivetrain;
    }
    if (drivetrain_texture) {
        static const SDL_FRect unit_rect = {static_cast<float>(WIDTH - SPRITE_WIDTH), 0, static_cast<float>(SPRITE_WIDTH), static_cast<float>(SPRITE_HEIGHT)};
        SDL_RenderTexture(renderer, drivetrain_texture, nullptr, &unit_rect);
    }
}

static void draw_class_id(int id) {
    static SDL_Texture* id_texture = nullptr;
    static int last_id = -1;
    if (last_id !=  id) {
        char buffer[3]{0};
        SDL_snprintf(buffer, sizeof(buffer), "%s", CLASS_IDS[id]);
        texture_text(renderer, &id_texture, buffer, pi_font, WHITE);
        last_id =id;
    }
    if (id_texture) {
        // Let ID take up 40 % of the space on the left
        static const float ID_SPACE = 0.4f;
        static const SDL_FRect class_bg = { 0, 0, static_cast<float>(WIDTH - SPRITE_WIDTH) * ID_SPACE, static_cast<float>(SPRITE_HEIGHT)};
        static const SDL_FRect rest_bg = {static_cast<float>(WIDTH - SPRITE_WIDTH) * ID_SPACE, 0, static_cast<float>(WIDTH - SPRITE_WIDTH) * (1-ID_SPACE),static_cast<float>(SPRITE_HEIGHT)};

        SDL_SetRenderDrawColor(renderer, CLASS_COLORS[id].r, CLASS_COLORS[id].g, CLASS_COLORS[id].b, CLASS_COLORS[id].a);
        SDL_RenderFillRect(renderer, &class_bg);
        SDL_RenderFillRect(renderer, &rest_bg);

        const SDL_FRect text_rect = {
            class_bg.x + class_bg.w * 0.125f - ((id != 4 and id != 5 ) ? class_bg.w * 0.75f / 4 : 0),
            class_bg.y + class_bg.h * 0.125f,
            class_bg.w * 0.75f,
            class_bg.h * 0.75f
        };

        SDL_RenderTexture(renderer, id_texture, nullptr, &text_rect);
    }
}

static void draw_performance_index(int performance) {
    performance = std::clamp(performance,100,999); // Clamping only for g++. Value is always between 100,999
    static SDL_Texture* performance_texture = nullptr;
    static int last_performance = -1;

    if (last_performance !=  performance) {
        char buffer[4]{0};
        SDL_snprintf(buffer, sizeof(buffer), "%d", performance);
        texture_text(renderer, &performance_texture, buffer, pi_font, WHITE);
        last_performance = performance;
    }
    if (performance_texture) {
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

        SDL_RenderTexture(renderer, performance_texture, nullptr, &text_rect);
    }
}

static void draw_group(int group) {
    static auto group_map = car_group_map();
    static SDL_Texture* group_texture = nullptr;
    static int last_group = -1;
    if (last_group != group) {
        char buffer[TEXT_WIDTH]{' '};
        buffer[TEXT_WIDTH-1] = '\0';
        SDL_snprintf(buffer, sizeof(buffer), "%-24s", group_map[group].c_str());
        texture_text(renderer, &group_texture, buffer, text_font, WHITE);
        last_group = group;
    }
    if (group_texture) {
        const SDL_FRect unit_rect = { 0, static_cast<float>(SPRITE_HEIGHT), static_cast<float>(WIDTH-SPRITE_WIDTH), static_cast<float>(SPRITE_WIDTH / 3)};
        SDL_RenderTexture(renderer, group_texture, nullptr, &unit_rect);
    }
}

static void draw_year_make(int year, const std::string& make) {
    year = std::clamp(year,1000,9999);
    static auto group_map = car_group_map();
    static SDL_Texture* year_make_texture = nullptr;
    static int last_year = -1;
    static std::string last_make;
    if (last_make != make or last_year != year) {
        char buffer[TEXT_WIDTH]{' '};
        buffer[TEXT_WIDTH-1] = '\0';
        SDL_snprintf(buffer, sizeof(buffer), "%d - %-17s", year, make.c_str());
        texture_text(renderer, &year_make_texture, buffer, text_font, WHITE);
        last_make = make;
        last_year = year;
    }
    if (year_make_texture) {
        const SDL_FRect unit_rect = { 0, static_cast<float>(SPRITE_HEIGHT + SPRITE_WIDTH / 3), static_cast<float>(WIDTH-SPRITE_WIDTH), static_cast<float>(SPRITE_WIDTH / 3)};
        SDL_RenderTexture(renderer, year_make_texture, nullptr, &unit_rect);
    }
}

static void draw_model(const std::string& model) {
    static SDL_Texture* model_texture = nullptr;
    static std::string last_model;
    if (last_model != model) {
        char buffer[TEXT_WIDTH]{' '};
        buffer[TEXT_WIDTH-1] = '\0';
        SDL_snprintf(buffer, sizeof(buffer), "%-24s", model.c_str());
        texture_text(renderer, &model_texture, buffer, text_font, WHITE);
        last_model = model;
    }
    if (model_texture) {
        const SDL_FRect unit_rect = { 0, static_cast<float>(SPRITE_HEIGHT + SPRITE_WIDTH *2 / 3), static_cast<float>(WIDTH-SPRITE_WIDTH), static_cast<float>(SPRITE_WIDTH / 3)};
        SDL_RenderTexture(renderer, model_texture, nullptr, &unit_rect);
    }
}

static void draw_flag(std::string& country) {
    static SDL_Texture* country_texture = nullptr;
    static std::string last_country;
    if (last_country != country) {
        texture_png(renderer,&country_texture,std::format("assets/flags/{}.png", country).c_str());
        last_country = country;
    }
    if (country_texture) {
        static const SDL_FRect unit_rect = {WIDTH * (1 - SPRITE_PORTION), static_cast<float>(SPRITE_HEIGHT), static_cast<float>(SPRITE_WIDTH), static_cast<float>(SPRITE_WIDTH)};
        SDL_RenderTexture(renderer, country_texture, nullptr, &unit_rect);
    }
}

void car_info_t::update(const fh6_data& data_out) {
    if (data_out.CarPerformanceIndex == 0) {
        //In menu!
        return;
    }
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    const int car_id = data_out.CarOrdinal;
    static auto details_map = car_details_map();
    const auto details = details_map[car_id];

    static auto country_map = car_country_map();

    draw_class_id(data_out.CarClass);
    draw_performance_index(data_out.CarPerformanceIndex);
    draw_drivetrain(data_out.DrivetrainType);
    draw_group(data_out.CarGroup);
    draw_model(details.model);
    draw_year_make(details.year,details.make);
    draw_flag(country_map[details.make]);

    SDL_RenderPresent(renderer);
}

void car_info_t::close() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (pi_font) TTF_CloseFont(pi_font);
    if (text_font) TTF_CloseFont(text_font);
}

