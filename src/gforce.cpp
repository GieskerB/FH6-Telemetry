#include <cstdio>
#include <stdlib.h>
#include <algorithm>
#include <cmath>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <sstream>
#include <iomanip>
#include <cstring>

#include "../include/gforce.hpp"
#include "../include/util/colors.hpp"
#include "../include/util/texture_handler.hpp"

static unsigned short WIDTH;
static unsigned short HEIGHT;

static constexpr unsigned char HISTORY_SIZE = 69;
static constexpr int G_MAX = 4;
static constexpr int SPEED_MAX = G_MAX * 100;

static SDL_Window * window = nullptr;
static SDL_Renderer* renderer = nullptr;
static TTF_Font* font = nullptr;

void gforce_t::init(unsigned short size) {

    WIDTH = size;
    HEIGHT = size;

    window = SDL_CreateWindow("GForce",WIDTH,HEIGHT,SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
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
    font = TTF_OpenFont("assets/fonts/digital-7.ttf",64);
    if(font == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    mutex = std::make_unique<std::mutex>();
}

static void render_points(short head, const SDL_Point point, SDL_Point hist[], const SDL_Color& color) {
    hist[head] = point;
    int index = head--;
    for(int i = 0; i < HISTORY_SIZE; ++i) {
        auto & p = hist[index++];
        if(index >= HISTORY_SIZE) index = 0;;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255 - i * (255.f / HISTORY_SIZE));
        const SDL_FRect rect {p.x -3.f , p.y - 3.f,5,5};
        SDL_RenderFillRect(renderer, &rect);
    }
}

static void render_points(const SDL_Point& gforce_point, const SDL_Point& speed_point) {
    static short head = 0;
    static SDL_Point g_history[HISTORY_SIZE]{0,0};
    static SDL_Point s_history[HISTORY_SIZE]{0,0};

    render_points(head, gforce_point, g_history, ORANGE);
    render_points(head, speed_point, s_history, BLUE);

    if(--head < 0) head = HISTORY_SIZE-1;
}

static std::pair<SDL_Point, std::string> update_gforce_point(float acc_x, float acc_z,unsigned char& changed) {

    float x = -acc_x / 9.81f;
    float z = acc_z / 9.81f;
    // round to two decimal places
    const float gforce = static_cast<int> (100 * std::hypot(x,z)) / 100.f;
    if(gforce > G_MAX) {
        x = x / gforce * G_MAX;
        z = z / gforce * G_MAX;
    }

    //update even without change for correct history!
    changed |= 0b1;
    const SDL_Point new_point{WIDTH /2 + static_cast<int>((x / G_MAX) * WIDTH / 2),HEIGHT /2 + static_cast<int>((z / G_MAX) * HEIGHT / 2)};

    static float last_gforce -1;
    static std::string return_value{};
    if (gforce != last_gforce) {
        last_gforce = gforce;
        std::stringstream strstream;
        strstream << std::fixed << std::setprecision(2) << gforce;
        return_value = strstream.str();
        changed |= 0b100;
    }

    return {new_point,return_value};
}
static std::pair<SDL_Point, std::string> update_speed_point(float speed_x, float speed_z, unsigned char& changed) {


    float x = speed_x * 3.6f;
    float z = -speed_z * 3.6f;

    const float speed = std::hypot(x,z);

    if(speed > G_MAX * 100) {
        x = x / speed * SPEED_MAX;
        z = z / speed * SPEED_MAX;
    }

    //update even without change for correct history!
    changed |= 0b10;
    const SDL_Point new_point {WIDTH /2 + static_cast<int>((x / G_MAX) * WIDTH / 2),HEIGHT /2 + static_cast<int>((z / G_MAX) * HEIGHT / 2)};

    static float last_speed = -1;
    static std::string return_value{};
    if(speed != last_speed) {
        last_speed = speed;
        std::stringstream strstream;
        strstream << std::setw(3) << std::setfill('0') << speed;
        return_value = strstream.str();
        changed |= 0b1000;
    }

    return {new_point, return_value};

}

void gforce_t::update(const fh6_data& data_out) {
    const bool is_paused = data_out.PositionX == 0 and data_out.PositionY == 0 and data_out.PositionZ == 0;

    if(is_paused) {
        mutex->lock();
        data.is_paused = is_paused;
        mutex->unlock();
        return;
    }

    unsigned char changes = 0;
    auto gforce_pair = update_gforce_point(data_out.AccelerationX, data_out.AccelerationZ, changes);
    auto speed_pair = update_speed_point(data_out.VelocityX, data_out.VelocityZ, changes);

    if (changes != 0) {
        mutex->lock();
        data.is_paused = is_paused;
        data.new_data = changes;
        data.new_gforce_point = gforce_pair.first;
        data.new_speed_point = speed_pair.first;
        std::strncpy(data.gforce,gforce_pair.second.c_str(), sizeof(data.gforce)-1);
        std::strncpy(data.speed,gforce_pair.second.c_str(), sizeof(data.speed)-1);
        mutex->unlock();
    }

    // // Clear screen
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    // SDL_RenderClear(renderer);

    // render_background();
    // render_gforce_labels();
    // render_speed_labels();

    // float z = data_out.AccelerationZ / 9.81f;
    // float x = -data_out.AccelerationX / 9.81f;
    // // round to two decimal places
    // float combined = static_cast<int> (100 * std::sqrt(z * z + x * x)) / 100.f;

    // if(combined > G_MAX) {
    //     z = z / combined * G_MAX;
    //     x = x / combined * G_MAX;
    // }

    // const SDL_Point gforce_point {WIDTH /2 + static_cast<int>((x / G_MAX) * WIDTH /2), HEIGHT /2 + static_cast<int>((z / G_MAX) * HEIGHT /2)};
    // render_gforce(combined);

    // z = -data_out.VelocityZ * 3.6f;
    // x = data_out.VelocityX * 3.6f;
    // combined = std::sqrt(z * z + x * x);

    // if(combined > G_MAX * 100) {
    //     z = z / combined * SPEED_MAX;
    //     x = x / combined * SPEED_MAX;
    // }

    // const SDL_Point speed_point {WIDTH /2 + static_cast<int>((x / SPEED_MAX) * WIDTH /2),HEIGHT /2 + static_cast<int>((z / SPEED_MAX) * HEIGHT /2)};

    // render_speed(static_cast<int>(combined));
    // render_points(gforce_point, speed_point);

    // SDL_RenderPresent(renderer);
}

static void render_static_text() {
    ///
    static SDL_Texture* background_texture = nullptr;
    if (!background_texture) texture_png_static(renderer,&background_texture,static_background_path);
    if (background_texture) {
        static const SDL_FRect unit_rect = { 0, 0, static_cast<float>(WIDTH), static_cast<float>(HEIGHT)};
        SDL_RenderTexture(renderer, background_texture, nullptr, &unit_rect);
    }
}

static void render_background() {

}

static void render_gforce_labels() {

}

static void render_speed_labels() {
    static SDL_Texture* speed_labels[G_MAX];
    if (!speed_labels[0]) {
        for(int i = 0 ; i < G_MAX; ++i) {
            char buffer[4]{0};
            SDL_snprintf(buffer, sizeof(buffer), "%d", SPEED_MAX - i * 100);
            texture_text_static(renderer, &speed_labels[i], buffer, font, BLUE);
        }
    }
    if (speed_labels[0]) {
        for(int i = 0 ; i < G_MAX; ++i) {
            const SDL_FRect unit_rect = { (i) * WIDTH / 8.f, static_cast<float>(HEIGHT / 2), WIDTH * 0.075f, WIDTH * 0.05f };
            SDL_RenderTexture(renderer, speed_labels[i], nullptr, &unit_rect);
        }
    }
}

static void render_gforce(const char* gforce) {
    static SDL_Texture* texture = nullptr;
    texture_text(renderer, &texture, gforce, font, ORANGE);
    if (texture) {
        const SDL_FRect unit_rect = { WIDTH * 0.7f, HEIGHT * 0.92f, WIDTH * 0.3f, WIDTH * 0.08f };
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}

static void render_speed(const char* speed) {
    static SDL_Texture* texture = nullptr;
        texture_text(renderer, &texture, speed, font, BLUE);
    if (texture) {
        const SDL_FRect unit_rect = { 0, HEIGHT * 0.92f, WIDTH * 0.3f, WIDTH * 0.08f };
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}


void gforce_t::render() {
    gforce_data data_copy;
    mutex->lock();
    if (data.new_data == 0 or data.is_paused) {
        mutex->unlock();
        return;
    }
    std::memcpy(&data_copy,&data,sizeof(data));
    mutex->unlock();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    if (data_copy.new_data & 0b1) render_g(data_copy.gear);
    if (data_copy.new_data & 0b10) render_speed(data_copy.speed);
    if (data_copy.new_data & 0b100) render_gforce(data_copy.gforce);
    if (data_copy.new_data & 0b1000) render_speed(data_copy.speed);
    render_static_text(static_kmh_text);

    SDL_RenderPresent(renderer);
}

void gforce_t::close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
}
