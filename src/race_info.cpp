#include <cstdio>
#include <stdlib.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <sstream>
#include <iomanip>
#include <cstring>

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

static std::string format_time(float seconds) {
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
    std::stringstream strstream;
    strstream << hours << ':' << std::setw(2) << std::setfill('0') << minutes
                       << ':' << std::setw(2) << std::setfill('0') << secs
                       << '.' << std::setw(3) << std::setfill('0') << ms;
    return strstream.str();
}

static SDL_FRect calc_centered_rect(SDL_Texture* texture, float center_x, float center_y, float target_height) {
    float w, h;
    SDL_GetTextureSize(texture, &w, &h);

    const float ratio = w / h;
    const float final_width = target_height * ratio;

    return {
        center_x - (final_width / 2.f),
        center_y - (target_height / 2.f),
        final_width,
        target_height
    };
}

static std::string update_position(unsigned char position, unsigned char& changed) {
    static unsigned char last_position = -1;
    static std::string return_value{};
    if (position != last_position) {
        last_position = position;
        return_value = std::to_string(position);
        changed |= 0b1;
    }
    return return_value;
}
static std::string update_lap(unsigned short lap, unsigned char& changed) {
    static unsigned char last_lap = -1;
    static std::string return_value{};
    if (lap != last_lap) {
        last_lap = lap;
        return_value = std::to_string(lap);
        changed |= 0b10;
    }
    return return_value;
}
static std::string update_race_time(float race_time, unsigned char& changed) {
    static unsigned char last_race_time = -1;
    static std::string return_value{};
    if (race_time != last_race_time) {
        last_race_time = race_time;
        return_value = format_time(race_time);
        changed |= 0b100;
    }
    return return_value;
}
static std::string update_current_lap(float current_lap, unsigned char& changed) {
    static unsigned char last_current_lap = -1;
    static std::string return_value{};
    if (current_lap != last_current_lap) {
        last_current_lap = current_lap;
        return_value = format_time(current_lap);
        changed |= 0b1000;
    }
    return return_value;
}
static std::string update_best_lap(float best_lap, unsigned char& changed) {
    static float last_best_lap = -1;
    static std::string return_value{};
    if (best_lap != last_best_lap) {
        last_best_lap = best_lap;
        return_value = format_time(best_lap);
        changed |= 0b10000;
    }
    return return_value;
}
static std::string update_last_lap(float last_lap, unsigned char& changed) {
    static float last_last_lap = -1;
    static std::string return_value{};
    if (last_lap != last_last_lap) {
        last_last_lap = last_lap;
        return_value = format_time(last_lap);
        changed |= 0b100000;
    }
    return return_value;
}
static std::string update_distance(float distance, unsigned char& changed) {
    distance /= 1000;
    if (distance > 9999.9f) {
        distance = 9999.9f;
    }
    static float last_distance = -1;
    static std::string return_value{};
    if (distance != last_distance) {
        last_distance = distance;
        std::stringstream strstream;
        strstream << std::fixed << std::setprecision(2) << std::setw(7) << std:: setfill ('0') << distance;
        return_value = strstream.str();
        changed |= 0b1000000;
    }
    return return_value;
}
static std::string update_shifts(unsigned int shifts, unsigned char& changed) {
    if (shifts > 99999) {
        shifts = 99999;
    }
    static unsigned int last_shifts = -1;
    static std::string return_value{};
    if (shifts != last_shifts) {
        last_shifts = shifts;
        std::stringstream strstream;
        strstream << std::setw(5) << std:: setfill ('0') << shifts;
        return_value = strstream.str();
        changed |= 0b10000000;
    }
    return return_value;
}

void race_info_t::update(const fh6_data& data_out) {
    const bool is_paused = data_out.PositionX == 0 and data_out.PositionY == 0 and data_out.PositionZ == 0;

    if(is_paused) {
        mutex->lock();
        data.is_paused = is_paused;
        mutex->unlock();
        return;
    }

    const bool on_race = data_out.IsRaceOn;
    static bool was_on_race = false;
    static float race_start_time = 0;
    static float distant_at_start = 0;
    static unsigned short shift_count = 0;
    static int last_gear = 1;

    // std::cout << "[["<< +on_race <<  +was_on_race<<"]]";

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
        ++shift_count;
    }

    unsigned char changes = 0;
    std::string position = update_position(data_out.RacePosition, changes);
    std::string lap = update_lap(data_out.LapNumber, changes);
    std::string race_time = update_race_time(data_out.CurrentRaceTime - race_start_time, changes);
    std::string current_lap = update_current_lap(data_out.CurrentLap, changes);
    std::string best_lap = update_best_lap(data_out.BestLap, changes);
    std::string last_lap = update_last_lap(data_out.LastLap, changes);
    std::string distance = update_distance(data_out.DistanceTraveled - distant_at_start, changes);
    std::string shifts = update_shifts(shift_count, changes);

    if (changes != 0) {
        mutex->lock();
        data.is_paused = is_paused;
        data.new_data = changes;
        std::strncpy(data.position,position.c_str(), sizeof(data.position)-1);
        std::strncpy(data.lap,lap.c_str(), sizeof(data.lap)-1);
        std::strncpy(data.race_time,race_time.c_str(), sizeof(data.race_time)-1);
        std::strncpy(data.current_lap,current_lap.c_str(), sizeof(data.current_lap)-1);
        std::strncpy(data.best_lap,best_lap.c_str(), sizeof(data.best_lap)-1);
        std::strncpy(data.last_lap,last_lap.c_str(), sizeof(data.last_lap)-1);
        std::strncpy(data.distance,distance.c_str(), sizeof(data.distance)-1);
        std::strncpy(data.shifts,shifts.c_str(), sizeof(data.shifts)-1);
        mutex->unlock();
    }
}

static void render_static_position_text() {
    static SDL_Texture* texture = nullptr;
    if (texture == nullptr) texture_text_static(renderer, &texture, static_position_text, text_font, WHITE);
    if (texture) {
        static const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.225f, HEIGHT * 0.05f, HEIGHT * 0.075f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_static_lap_text() {
    static SDL_Texture* texture = nullptr;
    if (texture == nullptr) texture_text_static(renderer, &texture, static_lap_text, text_font, WHITE);
    if (texture) {
        static const SDL_FRect unit_rect = calc_centered_rect(texture,WIDTH * 0.8f, HEIGHT * 0.05f, HEIGHT * 0.075f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_static_race_time_text() {
    static SDL_Texture* texture = nullptr;
    if (texture == nullptr) texture_text_static(renderer, &texture, static_race_time_text, text_font, WHITE);
    if (texture) {
        static const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.50f, HEIGHT * 0.15f, HEIGHT * 0.075f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_static_current_lap_text() {
    static SDL_Texture* texture = nullptr;
    if (texture == nullptr) texture_text_static(renderer, &texture, static_current_lap_text, text_font, WHITE);
    if (texture) {
        static const SDL_FRect unit_rect = calc_centered_rect(texture,  WIDTH * 0.50f, HEIGHT * 0.375f, HEIGHT * 0.075f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_static_best_lap_text() {
    static SDL_Texture* texture = nullptr;
    if (texture == nullptr) texture_text_static(renderer, &texture, static_best_lap_text, text_font, WHITE);
    if (texture) {
        static const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.75f, HEIGHT * 0.6f, HEIGHT * 0.0525f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_static_last_lap_text() {
    static SDL_Texture* texture = nullptr;
    if (texture == nullptr) texture_text_static(renderer, &texture, static_last_lap_text, text_font, WHITE);
    if (texture) {
        static const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.25f, HEIGHT * 0.6f, HEIGHT * 0.0525f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_static_distance_text() {
    static SDL_Texture* texture = nullptr;
    if (texture == nullptr) texture_text_static(renderer, &texture, static_distance_text, text_font, WHITE);
    if (texture) {
        static const SDL_FRect unit_rect = calc_centered_rect(texture,  WIDTH * 0.25f, HEIGHT * 0.825f, HEIGHT * 0.0525f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_static_shifts_text() {
    static SDL_Texture* texture = nullptr;
    if (texture == nullptr) texture_text_static(renderer, &texture, static_shifts_text, text_font, WHITE);
    if (texture) {
        static const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.75f, HEIGHT * 0.825f, HEIGHT * 0.0525f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}

static void render_position(const char* position, bool changed) {
    static SDL_Texture* texture = nullptr;
    if(changed or !texture) texture_text(renderer, &texture, position, num_font, WHITE);
    if (texture) {
        const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.45f, HEIGHT * 0.05f, HEIGHT * 0.08f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_lap(const char* lap, bool changed) {
    static SDL_Texture* texture = nullptr;
    if(changed or !texture) texture_text(renderer, &texture, lap, num_font, WHITE);
    if (texture) {
        const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.925f, HEIGHT * 0.05f, HEIGHT * 0.08f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_race_time(const char* race_time, bool changed) {
    static SDL_Texture* texture = nullptr;
    if(changed or !texture) texture_text(renderer, &texture, race_time, num_font, WHITE);
    if (texture) {
        const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.50f, HEIGHT * 0.25f, HEIGHT * 0.125f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_current_lap(const char* current_lap, bool changed) {
    static SDL_Texture* texture = nullptr;
    if(changed or !texture) texture_text(renderer, &texture, current_lap, num_font, WHITE);
    if (texture) {
        const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.50f, HEIGHT * 0.475f, HEIGHT * 0.125f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_best_lap(const char* best_lap, bool changed) {
    static SDL_Texture* texture = nullptr;
    if(changed or !texture) texture_text(renderer, &texture, best_lap, num_font, WHITE);
    if (texture) {
        const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.75f, HEIGHT * 0.675f, HEIGHT * 0.0775f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_last_lap(const char* last_lap, bool changed) {
    static SDL_Texture* texture = nullptr;
    if(changed or !texture) texture_text(renderer, &texture, last_lap, num_font, WHITE);
    if (texture) {
        const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.25f, HEIGHT * 0.675f, HEIGHT * 0.0775f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_distance(const char* distance, bool changed) {
    static SDL_Texture* texture = nullptr;
    if(changed or !texture) texture_text(renderer, &texture, distance, num_font, WHITE);
    if (texture) {
        const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.25f, HEIGHT * 0.9f, HEIGHT * 0.0775f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}
static void render_shifts(const char* shifts, bool changed) {
    static SDL_Texture* texture = nullptr;
    if(changed or !texture) texture_text(renderer, &texture, shifts, num_font, WHITE);
    if (texture) {
        const SDL_FRect unit_rect = calc_centered_rect(texture, WIDTH * 0.75f, HEIGHT * 0.9f, HEIGHT * 0.0775f);
        SDL_RenderTexture(renderer, texture, nullptr, &unit_rect);
    }
}

void race_info_t::render() {
    race_info_data data_copy;
    mutex->lock();
    if (data.new_data == 0 or data.is_paused) {
        mutex->unlock();
        return;
    }
    std::memcpy(&data_copy,&data,sizeof(data));
    mutex->unlock();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    render_static_position_text();
    render_static_lap_text();
    render_static_race_time_text();
    render_static_current_lap_text();
    render_static_best_lap_text();
    render_static_last_lap_text();
    render_static_distance_text();
    render_static_shifts_text();

    render_position(data_copy.position, (data_copy.new_data & 0b1) == 0b1);
    render_lap(data_copy.lap, (data_copy.new_data & 0b10) == 0b10);
    render_race_time(data_copy.race_time, (data_copy.new_data & 0b100) == 0b100);
    render_current_lap(data_copy.current_lap, (data_copy.new_data & 0b1000) == 0b1000);
    render_best_lap(data_copy.best_lap, (data_copy.new_data & 0b10000) == 0b10000);
    render_last_lap(data_copy.last_lap, (data_copy.new_data & 0b100000) == 0b100000);
    render_distance(data_copy.distance, (data_copy.new_data & 0b1000000) == 0b1000000);
    render_shifts(data_copy.shifts, (data_copy.new_data & 0b10000000) == 0b10000000);

    SDL_RenderPresent(renderer);
}

void race_info_t::close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(text_font);
    TTF_CloseFont(num_font);
}
