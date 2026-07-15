#include "../include/wheel_info.hpp"

#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "../include/util/colors.hpp"
#include "../include/util/texture_handler.hpp"

static unsigned short WIDTH;
static unsigned short HEIGHT;

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static TTF_Font* font = nullptr;

void wheel_info_t::init(unsigned short size) {
    WIDTH = size;
    HEIGHT = static_cast<unsigned short>(size * 0.69f);

    window = SDL_CreateWindow("Wheel Info", WIDTH, HEIGHT, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
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
    font = TTF_OpenFont("assets/fonts/droid-sans.ttf", 100);
    if (font == nullptr) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

static const std::array<SDL_Color, 4>& update_slipping(float slips[4], unsigned short& changed) {
    static float last_slips[4]{-1};
    static std::array<SDL_Color, 4> return_value{{0, 0, 0, 255}};
    for (unsigned char i = 0; i < 4; ++i) {
        if (slips[i] > 1.1f) {
            slips[i] = 1.1f;
        }
        if (slips[i] != last_slips[i]) {
            last_slips[i] = slips[i];

            if (slips[i] < 0.9f) {
                return_value[i].r = 255 * slips[i] / 0.9f;
                return_value[i].g = 255;
                return_value[i].b = 0;
            } else if (slips[i] < 1.1f) {
                return_value[i].r = 255;
                return_value[i].g = 255 * (1 - (slips[i] - 0.9f) / 0.2f);
                return_value[i].b = 0;
            } else {
                return_value[i].r = 255;
                return_value[i].g = 0;
            }
            changed |= (0b1 << i);
        }
    }
    return return_value;
}
static const std::array<std::string, 4>& update_temperature(float temperature[4], unsigned short& changed) {
    static float last_temperatures[4]{0};
    static std::array<std::string, 4> return_value{};
    for (unsigned char i = 0; i < 4; ++i) {
        if (temperature[i] != last_temperatures[i]) {
            last_temperatures[i] = temperature[i];
            std::stringstream strstream;
            strstream << std::fixed << std::setprecision(1) << std::setw(5) << std::setfill(' ') << temperature[i] << "°C";
            return_value[i] = strstream.str();
            changed |= (0b10000 << i);
        }
    }
    return return_value;
}
static const std::array<std::string, 4>& update_wheel_speed(float rot_speed[4], float slip[4], char steer,
                                                            float car_speed, unsigned short& changed) {
    // Needs Tire Slip Ratio.
    // Needs estimated wheel diameter.

    static constexpr float alpha = 0.9f;

    static float estim_wheel_diam_front = 0;
    static float estim_wheel_diam_rear = 0;

    if (slip[0] < 1 and slip[1] < 1 and steer < 32 and steer > -32 and car_speed > 10) {
        // estimate front diameter
        const float diameter_fl = car_speed / rot_speed[0];
        const float diameter_fr = car_speed / rot_speed[1];
        const float avg_diameter = (diameter_fl + diameter_fr) / 2;
        estim_wheel_diam_front *= alpha;
        estim_wheel_diam_front += (1 - alpha) * avg_diameter;
    }

    if (slip[2] < 1 and slip[3] < 1 and car_speed > 10) {
        // estimate rear diameter
        const float diameter_rl = car_speed / rot_speed[2];
        const float diameter_rr = car_speed / rot_speed[3];
        const float avg_diameter = (diameter_rl + diameter_rr) / 2;
        estim_wheel_diam_rear *= alpha;
        estim_wheel_diam_rear += (1 - alpha) * avg_diameter;
    }

    int wheel_speed[4];
    for (int i = 0; i < 2; ++i) {
        wheel_speed[i] = estim_wheel_diam_front * rot_speed[i];
        wheel_speed[i + 2] = estim_wheel_diam_rear * rot_speed[i + 2];
    }

    static int last_wheel_speed[4]{-1};
    static std::array<std::string, 4> return_value{};
    for (int i = 0; i < 4; ++i) {
        if (wheel_speed[i] != last_wheel_speed[i]) {
            last_wheel_speed[i] = wheel_speed[i];
            std::stringstream strstream;
            strstream << wheel_speed[i];
            return_value[i] = strstream.str();
            changed |= 0b100000000 << 0b1;
        }
    }
    return return_value;
}
static const std::array<float, 4>& update_suspension(float suspension[4], unsigned short& changed) {
    static float last_suspension[4]{-1};
    static std::array<float, 4> return_value{};
    for (int i = 0; i < 4; ++i) {
        if (suspension[i] != last_suspension[i]) {
            last_suspension[i] = suspension[i];
            return_value[i] = suspension[i];
            changed |= 0b1000000000000 << 0b1;
        }
    }
    return return_value;
}

void wheel_info_t::update(const fh6_data& data_out) {
    const bool is_paused = data_out.PositionX == 0 and data_out.PositionY == 0 and data_out.PositionZ == 0;

    if (is_paused) {
        mutex->lock();
        data.is_paused = is_paused;
        mutex->unlock();
        return;
    }

    // For update_slipping
    float slips[4];
    slips[0] = data_out.TireCombinedSlipFrontLeft;
    slips[1] = data_out.TireCombinedSlipFrontRight;
    slips[2] = data_out.TireCombinedSlipRearLeft;
    slips[3] = data_out.TireCombinedSlipRearRight;

    // For update_temperature
    float temp[4]{0};
    temp[0] = data_out.TireTempFrontLeft;
    temp[1] = data_out.TireTempFrontRight;
    temp[2] = data_out.TireTempRearLeft;
    temp[3] = data_out.TireTempRearRight;

    // For Update_wheel_speed
    float rot_speed[4]{0}, slip[4]{0};
    rot_speed[0] = data_out.WheelRotationSpeedFrontLeft;
    rot_speed[1] = data_out.WheelRotationSpeedFrontRight;
    rot_speed[2] = data_out.WheelRotationSpeedRearLeft;
    rot_speed[3] = data_out.WheelRotationSpeedRearRight;
    slip[0] = data_out.TireSlipRatioFrontLeft;
    slip[1] = data_out.TireSlipRatioFrontRight;
    slip[2] = data_out.TireSlipRatioRearLeft;
    slip[3] = data_out.TireSlipRatioRearRight;

    // For update_suspension
    float suspend[4]{0};
    slip[0] = data_out.NormalizedSuspensionTravelFrontLeft;
    slip[1] = data_out.NormalizedSuspensionTravelFrontRight;
    slip[2] = data_out.NormalizedSuspensionTravelRearLeft;
    slip[3] = data_out.NormalizedSuspensionTravelRearRight;

    unsigned short changes = 0;
    const auto& slipping = update_slipping(slips, changes);
    const auto& temperature = update_temperature(temp, changes);
    const auto& wheel_speed = update_wheel_speed(rot_speed, slip, data_out.Steer, data_out.VelocityZ, changes);
    const auto& suspension = update_suspension(suspend, changes);

    if (changes != 0) {
        mutex->lock();
        data.is_paused = is_paused;
        data.new_data = changes;
        for (unsigned char i = 0; i < 4; ++i) {
            data.slipping[i] = slipping[i];
            std::strncpy(data.temperature[i], temperature[i].c_str(), sizeof(data.temperature[i]) - 1);
            std::strncpy(data.wheel_speed[i], wheel_speed[i].c_str(), sizeof(data.wheel_speed[i]) - 1);
            data.suspension[i] = suspension[i];
        }
        mutex->unlock();
    }
}

static void render_tires(const SDL_Color colors[4], unsigned short changed) {
    static SDL_Texture* texture[4]{nullptr};
    for (unsigned char i = 0; i < 4; ++i) {
        if (!texture[i]) texture_png(renderer, &texture[i], static_tire_path);
        if ((changed & (0b1 << i)) == (0b1 << i))
            SDL_SetTextureColorMod(texture[i], colors[i].r, colors[i].g, colors[i].b);
        if (texture[i]) {
            const SDL_FRect unit_rect{
                WIDTH * (0.02f + 0.5f * (i / 2)),    
                HEIGHT * (0.10f + 0.5f * (i % 2)),   
                WIDTH * 0.15f,                       
                HEIGHT * 0.30f                       
            };
            SDL_RenderTexture(renderer, texture[i], nullptr, &unit_rect);
        }
    }
}

static void render_temperature(char temperature[4][8], unsigned short changed) {
    static SDL_Texture* texture[4]{nullptr};
    for (unsigned char i = 0; i < 4; ++i) {
        if (!texture[i] or (changed & (0b10000 << i)) == (0b10000 << i)) texture_text(renderer, &texture[i], temperature[i], font, WHITE);
        if (texture[i]) {
            const SDL_FRect unit_rect{
                WIDTH * (0.26f + 0.5f * (i / 2)),    
                HEIGHT * (0.27f + 0.5f * (i % 2)),   
                WIDTH * 0.2f,                      
                HEIGHT * 0.1f                      
            };
            SDL_RenderTexture(renderer, texture[i], nullptr, &unit_rect);
        }
    }
}

static void render_speed(char speed[4][4], unsigned short changed) {
    static SDL_Texture* texture[4]{nullptr};
    for (unsigned char i = 0; i < 4; ++i) {
        if (!texture[i] or (changed & (0b10000 << i)) == (0b10000 << i)) texture_text(renderer, &texture[i], speed[i], font, WHITE);
        if (texture[i]) {
            const SDL_FRect unit_rect{
                WIDTH * (0.26f + 0.5f * (i / 2)),   
                HEIGHT * (0.10f + 0.5f * (i % 2)),  
                WIDTH * 0.2f,                       
                HEIGHT * 0.1f                       
            };
            SDL_RenderTexture(renderer, texture[i], nullptr, &unit_rect);
        }
    }
}


static void render_suspension(float travel[4], unsigned short changed) {
    static SDL_Texture* texture[4]{nullptr};
    for (unsigned char i = 0; i < 4; ++i) {
        if (!texture[i] or (changed & (0b10000 << i)) == (0b10000 << i)) texture_png(renderer, &texture[i],static_suspension_path);
        if (texture[i]) {
            const SDL_FRect unit_rect{
                WIDTH * (0.20f + 0.5f * (i / 2)) + travel[0] - travel[0], 
                HEIGHT * (0.10f + 0.5f * (i % 2)),                     
                WIDTH * 0.05f,                                          
                HEIGHT * 0.30f                                          
            };
            SDL_RenderTexture(renderer, texture[i], nullptr, &unit_rect);
        }
    }
}

void wheel_info_t::render() {
    wheel_info_data data_copy;
    mutex->lock();
    if (data.new_data == 0 or data.is_paused) {
        mutex->unlock();
        return;
    }
    std::memcpy(&data_copy, &data, sizeof(data));
    mutex->unlock();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    render_tires(data_copy.slipping, data_copy.new_data);
    render_temperature(data_copy.temperature, data_copy.new_data);
    render_speed(data_copy.wheel_speed, data_copy.new_data);
    render_suspension(data_copy.suspension, data_copy.new_data);

    SDL_RenderPresent(renderer);
}

void wheel_info_t::close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
}
