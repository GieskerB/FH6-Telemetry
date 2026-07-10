#include <cstdio>
#include <stdlib.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>

#include "../include/engine_rpm.hpp"
#include "../include/util/colors.hpp"
#include "../include/util/texture_handler.hpp"

static unsigned short WIDTH;
static unsigned short HEIGHT;

static constexpr unsigned short STEP_SIZE = 1000;

static SDL_Window * window = nullptr;
static SDL_Renderer* renderer = nullptr;
static TTF_Font* font = nullptr;

void engine_rpm_t::init(unsigned short size) {

    WIDTH = size;
    HEIGHT = size / 2;

    window = SDL_CreateWindow("Engine RPM",WIDTH,HEIGHT,SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
    if(window == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    renderer = SDL_CreateRenderer(window, NULL);
    if(renderer == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    font = TTF_OpenFont("assets/fonts/digital-7.ttf",255);
    if(font == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

static std::string update_gear(int gear, unsigned char& changed) {
    static std::string GEARS[] = {" R", " 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9", "10", " N"};
    static int last_gear = -1;
    static std::string return_value{};
    if (gear != last_gear) {
        last_gear = gear;
        return_value = GEARS[gear];
        changed |= 0b1;
    }
    return return_value;
}
static std::string update_speed(int speed, unsigned char& changed) {
    speed *= 3.6f; // m/s to km/h
    static int last_speed = -1;
    static std::string return_value{};
    if (speed != last_speed) {
        last_speed = speed;
        std::stringstream strstream;
        strstream << std::setw(3) << std::setfill('0') << speed;
        return_value = strstream.str();
        changed |= 0b10;
    }
    return return_value;
}
static SDL_Color update_rpm_bar_color(int current_rpm, int max_rpm, unsigned char& changed) {
    static constexpr float RPM_FADE = 0.6f;
    static int last_current_rpm = -1;
    static int last_max_rpm = -1;
    static SDL_Color return_value{0,0,0,0};
    if (current_rpm != last_current_rpm or max_rpm != last_max_rpm) {
        last_current_rpm = current_rpm;
        last_max_rpm = max_rpm;
        const float rpm_percentage = static_cast<float>(current_rpm) / max_rpm;
        unsigned char r = 255, g = 255;
        if (rpm_percentage < RPM_FADE) {
            // Fade from Green (0, 255, 0) to Yellow (255, 255, 0)
            const float factor = rpm_percentage / RPM_FADE;
            r = static_cast<unsigned char>(factor * 255.0f);
        } else {
            // Fade from Yellow (255, 255, 0) to Red (255, 0, 0)
            const float factor = (rpm_percentage - RPM_FADE) / (1-RPM_FADE);
            g = static_cast<unsigned char>((1.0f - factor) * 255.0f);
        }
        return_value = {r,g,0,255};
        changed |= 0b10000000;
    }
    return return_value;
}

void engine_rpm_t::update(const fh6_data & data_out) {
    const bool is_paused = data_out.PositionX == 0 and data_out.PositionY == 0 and data_out.PositionZ == 0;

    if(is_paused) {
        mutex->lock();
        data.is_paused = is_paused;
        mutex->unlock();
        return;
    }

    unsigned char changes = 0;
    std::string gear = update_gear(data_out.Gear, changes);
    std::string speed = update_speed(data_out.Speed, changes);
    SDL_Color rpm_bar_color = update_rpm_bar_color(data_out.CurrentEngineRpm, data_out.EngineMaxRpm, changes);

    if (changes != 0) {
        mutex->lock();
        data.is_paused = is_paused;
        data.new_data = changes;
        std::strncpy(data.gear,gear.c_str(), sizeof(data.gear)-1);
        std::strncpy(data.speed,speed.c_str(), sizeof(data.speed)-1);
        data.rpm_bar_color = rpm_bar_color;
        data.idle_rpm = data_out.EngineIdleRpm;
        data.current_rpm = data_out.CurrentEngineRpm;
        data.max_rpm = data_out.EngineMaxRpm;
        mutex->unlock();
    }
}

static void render_static_text() {
    static SDL_Texture* texture = nullptr;
    if (texture == nullptr) texture_text_static(renderer, &texture, static_kmh_text, font, WHITE);
    if (texture) {
        static const SDL_FRect unit_rect = { WIDTH * 0.75f, HEIGHT * 0.5f, WIDTH * 0.225f, HEIGHT * 0.2f };
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}

static void render_gear(const char* gear, bool changed) {
    static SDL_Texture* texture = nullptr;
    if(changed or !texture) texture_text(renderer, &texture, gear, font, ORANGE);
    if (texture) {
        static const SDL_FRect gear_rect = { WIDTH * 0.75f, HEIGHT * 0.1f, WIDTH * 0.225f, HEIGHT * 0.4f };
        SDL_RenderTexture(renderer, texture, nullptr, &gear_rect);
    }
}
static void render_speed(const char* speed, bool changed) {
    static SDL_Texture* texture = nullptr;
    if(changed or !texture) texture_text(renderer, &texture, speed, font, WHITE);
    if (texture) {
        static const SDL_FRect speed_rect = { HEIGHT * 0.1f, 0.0f, WIDTH * 0.667f, HEIGHT * 0.8f };
        SDL_RenderTexture(renderer, texture, nullptr, &speed_rect);
    }
}
static void render_rpm_bar(int idle_rpm, int current_rpm, int max_rpm, const SDL_Color& color) {
    static  const SDL_FRect full_bar = {WIDTH * 0.05f, HEIGHT * 0.75f, WIDTH * 0.9f, WIDTH * 0.1f};

    // Draw the Background
    SDL_SetRenderDrawColor(renderer, GRAY.r, GRAY.g, GRAY.b, 69);
    SDL_RenderFillRect(renderer, &full_bar);

    // Draw the Gradient Filled Bar
    const float rpm_percentage = static_cast<float>(current_rpm) / max_rpm;

    const float fill_width = full_bar.w * rpm_percentage;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_FRect fill_bar = {full_bar.x, full_bar.y, fill_width, full_bar.h};
    SDL_RenderFillRect(renderer, &fill_bar);


    // RPM Markers (1k, 2k, 3k...)
    const float pixels_per_rpm = full_bar.w / max_rpm;
    SDL_SetRenderDrawColor(renderer, LIGHT_GRAY.r, LIGHT_GRAY.g, LIGHT_GRAY.b, 255);
    for (int rpm_mark = STEP_SIZE; rpm_mark < max_rpm; rpm_mark += STEP_SIZE) {
        float mark_x = full_bar.x + (rpm_mark * pixels_per_rpm);
        SDL_RenderLine(renderer, mark_x, full_bar.y, mark_x, full_bar.y + (full_bar.h * 0.333f));
    }

    // Idle RPM Marker
    if (idle_rpm > 0.0f && idle_rpm < max_rpm) {
        float idle_x = full_bar.x + (idle_rpm * pixels_per_rpm);
        SDL_SetRenderDrawColor(renderer, BLUE.r, BLUE.g, BLUE.b, 255);
        SDL_RenderLine(renderer, idle_x, full_bar.y, idle_x, full_bar.y + full_bar.h);
    }

    // Draw the Outer Border Outline (White)
    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, 255);
    SDL_RenderRect(renderer, &full_bar);

}

void engine_rpm_t::render() {
    engine_rpm_data data_copy;
    mutex->lock();
    if (data.new_data == 0 or data.is_paused) {
        mutex->unlock();
        return;
    }
    std::memcpy(&data_copy,&data,sizeof(data));
    mutex->unlock();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    render_static_text();

    render_gear(data_copy.gear, (data_copy.new_data & 0b1) == 0b1);
    render_speed(data_copy.speed, (data_copy.new_data & 0b10) == 0b10);
    render_rpm_bar(data_copy.idle_rpm, data_copy.current_rpm, data_copy.max_rpm, data_copy.rpm_bar_color);

    SDL_RenderPresent(renderer);
}

void engine_rpm_t::close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
}
