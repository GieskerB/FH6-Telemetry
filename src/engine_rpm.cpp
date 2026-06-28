#include <cstdio>
#include <stdlib.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>

#include "../include/engine_rpm.hpp"
#include "../include/util/colors.hpp"
#include "../include/util/texture_handler.hpp"

static unsigned short WIDTH;
static unsigned short HEIGHT;

static constexpr unsigned short STEP_SIZE = 1000;

static constexpr float RPM_FADE = 0.6f;

static const char* GEARS[] = {" R", " 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9", "10", " N"};

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

static void draw_gear(int gear) {
    static SDL_Texture* gear_texture = nullptr;
    static int last_gear = -1;
    if (last_gear != gear) {
        char buffer[3]{0};
        SDL_snprintf(buffer, sizeof(buffer), "%s", GEARS[gear]);
        texture_text(renderer, &gear_texture, buffer, font, ORANGE);
        last_gear = gear;
    }
    if (gear_texture) {
        static const SDL_FRect gear_rect = { WIDTH * 0.75f, HEIGHT * 0.1f, WIDTH * 0.225f, HEIGHT * 0.4f };
        SDL_RenderTexture(renderer, gear_texture, nullptr, &gear_rect);
    }
}

static void draw_speed(int speed) {
    speed = std::clamp(static_cast<int>(std::abs(speed * 3.6f)), 0, 999);
    static SDL_Texture* speed_texture = nullptr;
    static int last_speed = -1;
    if (last_speed != speed) {
        char buffer[4]{0};
        SDL_snprintf(buffer, sizeof(buffer), "%03d", speed);
        texture_text(renderer, &speed_texture, buffer, font, WHITE);
        last_speed = speed;
    }
    if (speed_texture) {
        static const SDL_FRect speed_rect = { HEIGHT * 0.1f, 0.0f, WIDTH * 0.667f, HEIGHT * 0.8f };
        SDL_RenderTexture(renderer, speed_texture, nullptr, &speed_rect);
    }
}

static void draw_static_text() {
    static SDL_Texture* kmh_texture = nullptr;
    if (!kmh_texture) {
        char buffer[5]{0};
        SDL_snprintf(buffer, sizeof(buffer), "%s", "KM/H");
        texture_text_static(renderer, &kmh_texture, buffer, font, WHITE);
    }
    if (kmh_texture) {
        static const SDL_FRect unit_rect = { WIDTH * 0.75f, HEIGHT * 0.5f, WIDTH * 0.225f, HEIGHT * 0.2f };
        SDL_RenderTexture(renderer, kmh_texture, nullptr, &unit_rect);
    }
}

static void draw_rpm(float idle_rpm, float current_rpm, float max_rpm) {
    static  const SDL_FRect full_bar = {WIDTH * 0.05f, HEIGHT * 0.75f, WIDTH * 0.9f, WIDTH * 0.1f};

    // Draw the Background
    SDL_SetRenderDrawColor(renderer, GRAY.r, GRAY.g, GRAY.b, 69);
    SDL_RenderFillRect(renderer, &full_bar);

    // Draw the Gradient Filled Bar
    const float rpm_percentage = std::clamp(current_rpm / max_rpm, 0.f,1.f);

    const float fill_width = full_bar.w * rpm_percentage;

    unsigned char r = 0, g = 0;

    if (rpm_percentage < RPM_FADE) {
        // Fade from Green (0, 255, 0) to Yellow (255, 255, 0)
        const float factor = rpm_percentage / RPM_FADE;
        r = static_cast<unsigned char>(factor * 255.0f);
        g = 255;
    } else {
        // Fade from Yellow (255, 255, 0) to Red (255, 0, 0)
        const float factor = (rpm_percentage - RPM_FADE) / (1-RPM_FADE);
        r = 255;
        g = static_cast<unsigned char>((1.0f - factor) * 255.0f);
    }

    SDL_SetRenderDrawColor(renderer, r, g, 0, 255);
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

void engine_rpm_t::update(const fh6_data & data_out) {
    if(data_out.CurrentEngineRpm == 0) {
        const_cast<fh6_data &>(data_out).Gear = 11; // Neutral instead of reverse!
    }

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // Render all the information
    draw_static_text();
    draw_gear(data_out.Gear);
    draw_speed(data_out.Speed);
    draw_rpm(data_out.EngineIdleRpm, data_out.CurrentEngineRpm, data_out.EngineMaxRpm);
    SDL_RenderPresent(renderer);
}

void engine_rpm_t::close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
}
