#include "../include/gforce.hpp"

#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "../include/util/colors.hpp"
#include "../include/util/texture_handler.hpp"

static unsigned short WIDTH;
static unsigned short HEIGHT;

static constexpr unsigned char HISTORY_SIZE = 69;
static constexpr int G_MAX = 4;
static constexpr int SPEED_MAX = G_MAX * 100;

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static TTF_Font* font = nullptr;

void gforce_t::init(unsigned short size) {
    WIDTH = size;
    HEIGHT = size;

    window = SDL_CreateWindow("GForce", WIDTH, HEIGHT, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
    if (window == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    font = TTF_OpenFont("assets/fonts/digital-7.ttf", 64);
    if (font == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

static const std::pair<SDL_Point, std::string>& update_gforce_point(float acc_x, float acc_z, unsigned char& changed) {
    float x = -acc_x / 9.81f;
    float z = acc_z / 9.81f;
    // round to two decimal places
    const float gforce = static_cast<int>(100 * std::hypot(x, z)) / 100.f;
    if (gforce > G_MAX) {
        x = x / gforce * G_MAX;
        z = z / gforce * G_MAX;
    }

    static std::pair<SDL_Point, std::string> return_value{};

    return_value.first = {WIDTH / 2 + static_cast<int>((x / G_MAX) * WIDTH / 2),
                          HEIGHT / 2 + static_cast<int>((z / G_MAX) * HEIGHT / 2)};
    changed |= 0b10000000;

    static float last_gforce = -1;
    if (gforce != last_gforce) {
        last_gforce = gforce;
        std::stringstream strstream;
        strstream << std::fixed << std::setprecision(2) << gforce << 'G';
        return_value.second = strstream.str();
        changed |= 0b1;
    }

    return return_value;
}
static const std::pair<SDL_Point, std::string>& update_speed_point(float speed_x, float speed_z,
                                                                   unsigned char& changed) {
    float x = speed_x * 3.6f;
    float z = -speed_z * 3.6f;

    const int speed = static_cast<int>(std::round(std::hypot(x, z)));

    if (speed > SPEED_MAX) {
        x = x / speed * SPEED_MAX;
        z = z / speed * SPEED_MAX;
    }

    static std::pair<SDL_Point, std::string> return_value;

    return_value.first = {WIDTH / 2 + static_cast<int>((x / SPEED_MAX) * WIDTH / 2),
                          HEIGHT / 2 + static_cast<int>((z / SPEED_MAX) * HEIGHT / 2)};
    changed |= 0b10000000;

    static int last_speed = -1;
    if (speed != last_speed) {
        last_speed = speed;
        std::stringstream strstream;
        strstream << std::setw(3) << std::setfill('0') << speed << "KM/H";
        return_value.second = strstream.str();
        changed |= 0b10;
    }

    return return_value;
}

void gforce_t::update(const fh6_data& data_out) {
    const bool is_paused = data_out.PositionX == 0 and data_out.PositionY == 0 and data_out.PositionZ == 0;

    if (is_paused) {
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
        std::strncpy(data.gforce, gforce_pair.second.c_str(), sizeof(data.gforce) - 1);
        std::strncpy(data.speed, speed_pair.second.c_str(), sizeof(data.speed) - 1);
        mutex->unlock();
    }
}

static void render_static_background() {
    static SDL_Texture* texture = nullptr;
    if (!texture) texture_png_static(renderer, &texture, static_background_path);
    if (texture) {
        static const SDL_FRect unit_rect = {0, 0, static_cast<float>(WIDTH), static_cast<float>(HEIGHT)};
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_static_gforce_labels() {
    static SDL_Texture* textures[G_MAX];
    if (!textures[0]) {
        for (int i = 0; i < G_MAX; ++i) {
            texture_text_static(renderer, &textures[i], gforce_label[i], font, ORANGE);
        }
    }
    if (textures[0]) {
        for (int i = 0; i < G_MAX; ++i) {
            const SDL_FRect unit_rect = {WIDTH / 2.f + (i + 0.5f) * WIDTH / 8.f, static_cast<float>(HEIGHT / 2),
                                         WIDTH * 0.05f, WIDTH * 0.05f};
            SDL_RenderTexture(renderer, textures[i], nullptr, &unit_rect);
        }
    }
}
static void render_static_speed_labels() {
    static SDL_Texture* textures[G_MAX];
    if (!textures[0]) {
        for (int i = 0; i < G_MAX; ++i) {
            texture_text_static(renderer, &textures[i], speed_label[i], font, BLUE);
        }
    }
    if (textures[0]) {
        for (int i = 0; i < G_MAX; ++i) {
            const SDL_FRect unit_rect = {(i)*WIDTH / 8.f, static_cast<float>(HEIGHT / 2), WIDTH * 0.075f,
                                         WIDTH * 0.05f};
            SDL_RenderTexture(renderer, textures[i], nullptr, &unit_rect);
        }
    }
}

static void render_gforce_point(const SDL_Point& point) {
    static short head = 0;
    static SDL_Point history[HISTORY_SIZE]{0, 0};

    history[head] = point;
    int index = head--;
    for (int i = 0; i < HISTORY_SIZE; ++i) {
        auto& p = history[index++];
        if (index >= HISTORY_SIZE) index = 0;
        ;
        SDL_SetRenderDrawColor(renderer, ORANGE.r, ORANGE.g, ORANGE.b, 255 - i * (255.f / HISTORY_SIZE));
        const SDL_FRect rect{p.x - 3.f, p.y - 3.f, 5, 5};
        SDL_RenderFillRect(renderer, &rect);
    }

    if (head < 0) head = HISTORY_SIZE - 1;
}
static void render_speed_point(const SDL_Point& point) {
    static short head = 0;
    static SDL_Point history[HISTORY_SIZE]{0, 0};

    history[head] = point;
    int index = head--;
    for (int i = 0; i < HISTORY_SIZE; ++i) {
        auto& p = history[index++];
        if (index >= HISTORY_SIZE) index = 0;
        ;
        SDL_SetRenderDrawColor(renderer, BLUE.r, BLUE.g, BLUE.b, 255 - i * (255.f / HISTORY_SIZE));
        const SDL_FRect rect{p.x - 3.f, p.y - 3.f, 5, 5};
        SDL_RenderFillRect(renderer, &rect);
    }

    if (head < 0) head = HISTORY_SIZE - 1;
}
static void render_gforce(const char* gforce, bool changed) {
    static SDL_Texture* texture = nullptr;
    if (changed or !texture) texture_text(renderer, &texture, gforce, font, ORANGE);
    if (texture) {
        const SDL_FRect unit_rect = {WIDTH * 0.7f, HEIGHT * 0.92f, WIDTH * 0.3f, WIDTH * 0.08f};
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_speed(const char* speed, bool changed) {
    static SDL_Texture* texture = nullptr;
    if (changed or !texture) texture_text(renderer, &texture, speed, font, BLUE);
    if (texture) {
        const SDL_FRect unit_rect = {0, HEIGHT * 0.92f, WIDTH * 0.3f, WIDTH * 0.08f};
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
    std::memcpy(&data_copy, &data, sizeof(data));
    mutex->unlock();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    render_gforce_point(data_copy.new_gforce_point);
    render_speed_point(data_copy.new_speed_point);
    render_gforce(data_copy.gforce, (data_copy.new_data & 0b1) == 0b1);
    render_speed(data_copy.speed, (data_copy.new_data & 0b10) == 0b10);
    render_static_background();
    render_static_gforce_labels();
    render_static_speed_labels();

    SDL_RenderPresent(renderer);
}

void gforce_t::close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
}
