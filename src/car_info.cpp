#include <cstdio>
#include <stdlib.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>

#include "../include/car_info.hpp"
#include "../include/util/colors.hpp"
#include "../include/util/texture_handler.hpp"


static constexpr float SPRITE_RATIO = 197.f / 125.f;
static constexpr float SPRITE_PORTION = 0.25f;
static unsigned short WIDTH;
static unsigned short HEIGHT;
static unsigned short PADDING;

static constexpr SDL_Color CLASS_COLORS[8] = {{103, 185, 238, 255}, {246, 198, 85,255},
                                                {236, 109, 65, 255}, {233, 61, 78, 255},
                                                {172, 100, 224, 255}, {49, 93, 210, 255},
                                                {195, 53, 151, 255}, {101, 212, 104, 255}};

static const char* CLASS_IDS[] = {" D", " C", " B", " A", "S1", "S2", " R", " X"};
static const char* DRIVETRAIN_PNGS[] = {"assets/sprites/FWD.png","assets/sprites/RWD.png","assets/sprites/AWD.png"};

static SDL_Window * window = nullptr;
static SDL_Renderer* renderer = nullptr;
static TTF_Font* font = nullptr;

void car_info_t::init(unsigned short size) {

    WIDTH = size;
    HEIGHT = WIDTH * SPRITE_PORTION * SPRITE_RATIO;
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
    font = TTF_OpenFont("assets/fonts/digital-7.ttf",255);
    if(font == nullptr) {
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
        static const SDL_FRect unit_rect = {WIDTH * (1 - SPRITE_PORTION), 0, WIDTH * SPRITE_PORTION, static_cast<float>(HEIGHT)};
        SDL_RenderTexture(renderer, drivetrain_texture, nullptr, &unit_rect);
    }
}

static void draw_class_id(int id) {
    static SDL_Texture* id_texture = nullptr;
    static int last_id = -1;
    if (last_id !=  id) {
        texture_text<3,const char*>(renderer, &id_texture, "%s", CLASS_IDS[id], font, WHITE);
        last_id =id;
    }
    if (id_texture) {
        // Let ID take up 40 % of the space on the left
        static const SDL_FRect class_bg = { 0, 0, WIDTH * (1-SPRITE_PORTION) * 0.4f, static_cast<float>(HEIGHT)};
        static const SDL_FRect rest_bg = {WIDTH * (1-SPRITE_PORTION) * 0.4f, 0, WIDTH * (1-SPRITE_PORTION) * 0.6f,static_cast<float>(HEIGHT)};

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
        texture_text<4,int>(renderer, &performance_texture, "%d", performance, font, WHITE);
        last_performance = performance;
    }
    if (performance_texture) {
        // Let PI take up 60 % of the space on the right - some small padding
        static const SDL_FRect performance_bg = {
            WIDTH * (1-SPRITE_PORTION) * 0.4f + PADDING,
            static_cast<float>(PADDING),
            WIDTH * (1-SPRITE_PORTION) * 0.6f -2* PADDING,
            static_cast<float>(HEIGHT - 2 * PADDING)};

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

void car_info_t::update(const fh6_data& data_out) {
    if (data_out.CarPerformanceIndex == 0) {
        //In menu!
        return;
    }
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    draw_class_id(data_out.CarClass);
    draw_performance_index(data_out.CarPerformanceIndex);
    draw_drivetrain(data_out.DrivetrainType);

    SDL_RenderPresent(renderer);
}

void car_info_t::close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
}

