#include <cstdio>
#include <stdlib.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "../include/race_info.hpp"
#include "../include/util/colors.hpp"
#include "../include/util/texture_handler.hpp"

static unsigned short WIDTH;
static unsigned short HEIGHT;

static SDL_Window * window = nullptr;
static SDL_Renderer* renderer = nullptr;
static TTF_Font* text_font = nullptr;
static TTF_Font* num_font = nullptr;

void race_info_t::init(unsigned short size) {

    WIDTH = size;
    HEIGHT = static_cast<unsigned short>(size * 1.f);

    window = SDL_CreateWindow("Race Info",WIDTH,HEIGHT,SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
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
    text_font = TTF_OpenFont("assets/fonts/droid-sans.ttf",100);
    if(text_font == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    num_font = TTF_OpenFont("assets/fonts/digital-7.ttf",100);
    if(num_font == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

static void format_time(float seconds, char* buffer, size_t buffer_size) {
    if (seconds < 0.f) seconds = 0.f;

    int total_seconds = static_cast<int>(seconds);
    int ms = static_cast<int>((seconds - total_seconds) * 1000.f);
    int hours = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int secs = total_seconds % 60;

    // Clamp maximum display hours to 9
    if (hours > 9) {
        hours = 9; minutes = 59; secs = 59; ms = 999;
    }

    SDL_snprintf(buffer, buffer_size, "%1d:%02d:%02d.%03d", hours, minutes, secs, ms);
}

static void render_texture_fixed(SDL_Renderer* renderer, SDL_Texture* texture, float centerX, float centerY, float targetHeight) {
    if (!texture) return;

    float w, h;
    SDL_GetTextureSize(texture, &w, &h);

    float aspect_ratio = w / h;
    float final_width = targetHeight * aspect_ratio;

    SDL_FRect dest_rect {
        centerX - (final_width / 2.f),
        centerY - (targetHeight / 2.f),
        final_width,
        targetHeight
    };

    SDL_RenderTexture(renderer, texture, nullptr, &dest_rect);
}

static void draw_position_and_lap(unsigned char position, unsigned short lap_number) {
    static SDL_Texture* position_text_texture = nullptr;
    static SDL_Texture* lap_text_texture = nullptr;
    if (!position_text_texture) {
        texture_text_static(renderer, &position_text_texture, "Position:", text_font, WHITE);
        texture_text_static(renderer, &lap_text_texture, "Lap:", text_font, WHITE);
    }

    static SDL_Texture* position_texture = nullptr;
    static SDL_Texture* lap_texture = nullptr;
    static unsigned char  last_position = -1;
    static unsigned short last_lap = -1;

    if (last_position != position) {
        char buffer[3];
        SDL_snprintf(buffer, sizeof(buffer), "%02d", position);
        texture_text(renderer, &position_texture, buffer, num_font, ORANGE);
        last_position = position;
    }
    if (last_lap != lap_number) {
        char buffer[3];
        SDL_snprintf(buffer, sizeof(buffer), "%02d", lap_number);
        texture_text(renderer, &lap_texture, buffer, num_font, ORANGE);
        last_lap = lap_number;
    }

    render_texture_fixed(renderer, position_text_texture, WIDTH * 0.225f, HEIGHT * 0.05f, HEIGHT * 0.075f);
    render_texture_fixed(renderer, position_texture,   WIDTH * 0.45f, HEIGHT * 0.05f, HEIGHT * 0.078f);
    render_texture_fixed(renderer, lap_text_texture, WIDTH * 0.8f, HEIGHT * 0.05f, HEIGHT * 0.075f);
    render_texture_fixed(renderer, lap_texture, WIDTH * 0.925f, HEIGHT * 0.05f, HEIGHT * 0.08f);
}

static void draw_race_time(float race_time) {
    static SDL_Texture* total_time_text_texture = nullptr;
    if (!total_time_text_texture) {
        texture_text_static(renderer, &total_time_text_texture, "Total race time", text_font, WHITE);
    }

    static SDL_Texture* total_time_texture = nullptr;

    const int total_time_ms = static_cast<int>(race_time * 1000.f);
    static int last_time = -1;
    if (last_time != total_time_ms) {
        char buffer[12];
        format_time(race_time, buffer, sizeof(buffer));
        texture_text(renderer, &total_time_texture, buffer, num_font, WHITE);
        last_time = total_time_ms;
    }

    render_texture_fixed(renderer, total_time_text_texture, WIDTH * 0.50f, HEIGHT * 0.15f, HEIGHT * 0.075f);
    render_texture_fixed(renderer, total_time_texture,   WIDTH * 0.50f, HEIGHT * 0.25f, HEIGHT * 0.125f);
}

static void draw_current_lap(float current_lap_time) {
    static SDL_Texture* curr_time_text_texture = nullptr;
    if (!curr_time_text_texture) {
        texture_text_static(renderer, &curr_time_text_texture, "Current lap", text_font, BLUE);
    }

    static SDL_Texture* current_time_texture = nullptr;

    const int curr_lap_ms = static_cast<int>(current_lap_time * 1000.f);
    static int last_time = -1;
    if (last_time != curr_lap_ms) {
        char buffer[12];
        format_time(current_lap_time, buffer, sizeof(buffer));
        texture_text(renderer, &current_time_texture, buffer, num_font, BLUE);
        last_time = curr_lap_ms;
    }

    render_texture_fixed(renderer, curr_time_text_texture, WIDTH * 0.50f, HEIGHT * 0.375f, HEIGHT * 0.075f);
    render_texture_fixed(renderer, current_time_texture,   WIDTH * 0.50f, HEIGHT * 0.475f, HEIGHT * 0.125f);
}

static void draw_lap_history(float last_lap_time, float best_lap_time) {
    static SDL_Texture* last_lab_text_texture = nullptr;
    static SDL_Texture* best_lab_text_texture = nullptr;
    if (!last_lab_text_texture) {
        texture_text_static(renderer, &last_lab_text_texture, "Last lap", text_font, WHITE);
        texture_text_static(renderer, &best_lab_text_texture, "Best lap", text_font, GREEN);
    }

    static SDL_Texture* tex_last_lap = nullptr;
    static SDL_Texture* tex_best_lap = nullptr;

    const int last_lap_ms = static_cast<int>(last_lap_time * 1000.f);
    static int last_last_lap_ms = -1;
    if (last_last_lap_ms != last_lap_ms) {
        char buffer[16];
        format_time(last_lap_time, buffer, sizeof(buffer));
        texture_text(renderer, &tex_last_lap, buffer, num_font, WHITE);
        last_last_lap_ms = last_lap_ms;
    }

    const int best_lap_ms = static_cast<int>(best_lap_time * 1000.f);
    static int last_best_lap_ms = -1;
    if (last_best_lap_ms != best_lap_ms) {
        char buffer[16];
        format_time(best_lap_time, buffer, sizeof(buffer));
        texture_text(renderer, &tex_best_lap, buffer, num_font, GREEN);
        last_best_lap_ms = best_lap_ms;
    }

    render_texture_fixed(renderer, last_lab_text_texture, WIDTH * 0.25f, HEIGHT * 0.6f, HEIGHT * 0.0525f);
    render_texture_fixed(renderer, tex_last_lap,   WIDTH * 0.25f, HEIGHT * 0.675f, HEIGHT * 0.0775f);
    render_texture_fixed(renderer, best_lab_text_texture, WIDTH * 0.75f, HEIGHT * 0.6f, HEIGHT * 0.0525f);
    render_texture_fixed(renderer, tex_best_lap,   WIDTH * 0.75f, HEIGHT * 0.675f, HEIGHT * 0.0775f);
}

static void draw_distance_and_shifts(float distance_traveled, unsigned int shift_count) {
    static SDL_Texture* distance_text_texture = nullptr;
    static SDL_Texture* shift_text_texure = nullptr;
    if (!distance_text_texture) {
        texture_text_static(renderer, &distance_text_texture, "Distance (KM)", text_font, WHITE);
        texture_text_static(renderer, &shift_text_texure, "Shift count", text_font, WHITE);
    }

    static SDL_Texture* distance_texture = nullptr;
    static SDL_Texture* shift_texture = nullptr;

    const int dist = static_cast<int>(distance_traveled * 100.f);
    static int last_dist = -1;
    if (last_dist != dist) {
        char buffer[16];
        float distance_km = std::clamp(distance_traveled / 1000.f, 0.f, 999.99f);
        SDL_snprintf(buffer, sizeof(buffer), "%06.2f", distance_km);
        texture_text(renderer, &distance_texture, buffer, num_font, WHITE);
        last_dist = dist;
    }

    static unsigned int last_shifts = -1;
    if (last_shifts != shift_count) {
        char buffer[8];
        unsigned int shifts = std::clamp(shift_count, 0u, 99999u);
        SDL_snprintf(buffer, sizeof(buffer), "%05u", shifts);
        texture_text(renderer, &shift_texture, buffer, num_font, WHITE);
        last_shifts = shift_count;
    }

    render_texture_fixed(renderer, distance_text_texture,   WIDTH * 0.25f, HEIGHT * 0.825f, HEIGHT * 0.0525f);
    render_texture_fixed(renderer, distance_texture,     WIDTH * 0.25f, HEIGHT * 0.9f, HEIGHT * 0.0775f);
    render_texture_fixed(renderer, shift_text_texure, WIDTH * 0.75f, HEIGHT * 0.825f, HEIGHT * 0.0525f);
    render_texture_fixed(renderer, shift_texture,   WIDTH * 0.75f, HEIGHT * 0.9f, HEIGHT * 0.0775f);
}

#include <iostream>

void race_info_t::update(const fh6_data& data_out) {
    if(data_out.PositionX == 0.f and data_out.PositionZ == 0.f) {
        // In menu!
        return;
    }

    const bool on_race = data_out.IsRaceOn;
    static bool was_on_race = false;
    static float race_start_time = 0;
    static float distant_at_start = 0;
    static unsigned short shift_count = 0;
    static int last_gear = 1;
    if (!on_race and !was_on_race){
        // No race!
        return;
    } else if(on_race and !was_on_race) {
        // Just entered race!
        was_on_race = true;
        race_start_time = data_out.CurrentRaceTime;
        distant_at_start = data_out.DistanceTraveled;
        shift_count = 0;
        last_gear = 1;
    } else if (!on_race and was_on_race) {
        // Just left race!
        was_on_race = false;
    }

    if(last_gear != data_out.Gear and data_out.Gear != 11) {
        last_gear = data_out.Gear;
        std::cout << +data_out.Gear << "\n";
        ++shift_count;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // Call individual UI modular render components
    draw_position_and_lap(data_out.RacePosition, data_out.LapNumber);
    draw_race_time(data_out.CurrentRaceTime - race_start_time);
    draw_current_lap(data_out.CurrentLap);
    draw_lap_history(data_out.LastLap, data_out.BestLap);
    draw_distance_and_shifts(data_out.DistanceTraveled - distant_at_start, shift_count);

    SDL_RenderPresent(renderer);
}


void race_info_t::close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(text_font);
    TTF_CloseFont(num_font);
}
