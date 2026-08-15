#include "../include/wheel_info.hpp"

#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>

#include <algorithm>
#include <array>
#include <cmath>
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
    // multi init check
    static bool initialized = false;
    if (initialized) {
        throw std::runtime_error("Cannot instanace wheel-info more then once!\n");
    }

    WIDTH = size;
    HEIGHT = static_cast<unsigned short>(size * 0.6f);

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
    initialized = true;
}

static const std::array<SDL_Color, 4>& update_slipping(float slips[4], unsigned int& changed) {
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
static const std::array<std::string, 4>& update_temperature(float temperature[4], unsigned int& changed) {
    static float last_temperatures[4]{0};
    static std::array<std::string, 4> return_value{};
    for (unsigned char i = 0; i < 4; ++i) {
        if (temperature[i] != last_temperatures[i]) {
            last_temperatures[i] = temperature[i];
            std::stringstream strstream;
            strstream << std::fixed << std::setprecision(1) << std::setw(5) << std::setfill(' ')
                      << (temperature[i] - 32) / 1.8f << "°C";
            return_value[i] = strstream.str();
            changed |= (0b10000 << i);
        }
    }
    return return_value;
}
static const std::array<std::string, 4>& update_wheel_speed(float rot_speed[4], float slip[4], char steer,
                                                            float car_speed, unsigned int& changed) {
    // Needs Tire Slip Ratio.
    // Needs estimated wheel diameter.

    static constexpr float alpha = 0.9f;

    static float estim_wheel_diam_front = 0;
    static float estim_wheel_diam_rear = 0;

    if (slip[0] < 1 and slip[1] < 1 and steer < 32 and steer > -32 and rot_speed[0] > 10 and rot_speed[1] > 10) {
        // estimate front diameter
        const float diameter_fl = car_speed / rot_speed[0];
        const float diameter_fr = car_speed / rot_speed[1];
        const float avg_diameter = (diameter_fl + diameter_fr) / 2;
        estim_wheel_diam_front *= alpha;
        estim_wheel_diam_front += (1 - alpha) * avg_diameter;
    }

    if (slip[2] < 1 and slip[3] < 1 and rot_speed[2] > 10 and rot_speed[3] > 10) {
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
            strstream << std::setw(3) << std::setfill(' ') << std::abs(wheel_speed[i]) << "km/h";
            return_value[i] = strstream.str();
            changed |= (0b100000000 << i);
        }
    }
    return return_value;
}
static const std::array<float, 4>& update_lateral_slip(float total_slip[4], float forward_slip[4],
                                                       unsigned int& changed) {
    static std::array<float, 4> return_value{};
    for (int i = 0; i < 4; ++i) {
        float lateral = std::pow(total_slip[i], 2) - std::pow(forward_slip[i], 2);
        const char sign = lateral > 0 ? 1 : -1;
        lateral = std::sqrt(std::abs(lateral)) * sign;
        lateral = std::clamp(lateral, -1.f, 1.f);
        if (lateral != return_value[i]) {
            return_value[i] = lateral;
            changed |= (0b1000000000000 << i);
        }
    }
    return return_value;
}
static const std::array<float, 4>& update_suspension(float suspension[4], unsigned int& changed) {
    static std::array<float, 4> return_value{};
    for (int i = 0; i < 4; ++i) {
        if (suspension[i] != return_value[i]) {
            // already normalized
            return_value[i] = suspension[i];
            changed |= (0b10000000000000000 << i);
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

    // For update_slipping & update_lateral_slip
    float total_slip[4];
    total_slip[0] = data_out.TireCombinedSlipFrontLeft;
    total_slip[1] = data_out.TireCombinedSlipFrontRight;
    total_slip[2] = data_out.TireCombinedSlipRearLeft;
    total_slip[3] = data_out.TireCombinedSlipRearRight;

    // For Update_wheel_speed
    float rot_speed[4]{0}, forward_slip[4]{0};
    rot_speed[0] = std::abs(data_out.WheelRotationSpeedFrontLeft);
    rot_speed[1] = std::abs(data_out.WheelRotationSpeedFrontRight);
    rot_speed[2] = std::abs(data_out.WheelRotationSpeedRearLeft);
    rot_speed[3] = std::abs(data_out.WheelRotationSpeedRearRight);
    forward_slip[0] = data_out.TireSlipRatioFrontLeft;
    forward_slip[1] = data_out.TireSlipRatioFrontRight;
    forward_slip[2] = data_out.TireSlipRatioRearLeft;
    forward_slip[3] = data_out.TireSlipRatioRearRight;

    // For update_temperature & update_lateral_slip
    float temp[4]{0};
    temp[0] = data_out.TireTempFrontLeft;
    temp[1] = data_out.TireTempFrontRight;
    temp[2] = data_out.TireTempRearLeft;
    temp[3] = data_out.TireTempRearRight;

    // For update_suspension
    float suspend[4]{0};
    suspend[0] = data_out.NormalizedSuspensionTravelFrontLeft;
    suspend[1] = data_out.NormalizedSuspensionTravelFrontRight;
    suspend[2] = data_out.NormalizedSuspensionTravelRearLeft;
    suspend[3] = data_out.NormalizedSuspensionTravelRearRight;

    unsigned int changes = 0;
    const auto& slipping = update_slipping(total_slip, changes);
    const auto& wheel_speed =
        update_wheel_speed(rot_speed, forward_slip, data_out.Steer, std::abs(data_out.VelocityZ * 3.6f), changes);
    const auto& temperature = update_temperature(temp, changes);
    const auto& lateral_slip = update_lateral_slip(total_slip, forward_slip, changes);
    const auto& suspension = update_suspension(suspend, changes);

    mutex->lock();
    data.is_paused = is_paused;
    data.new_data = changes;
    for (unsigned char i = 0; i < 4 and changes != 0; ++i) {
        data.slipping[i] = slipping[i];
        std::snprintf(data.temperature[i], sizeof(data.temperature[i]), "%s", temperature[i].c_str());
        std::snprintf(data.wheel_speed[i], sizeof(data.wheel_speed[i]), "%s", wheel_speed[i].c_str());
        data.lateral_slip[i] = lateral_slip[i];
        data.suspension[i] = suspension[i];
    }
    mutex->unlock();
}

static void render_static_speed_text() {
    static SDL_Texture* texture[4]{nullptr};
    for (int i = 0; i < 4; ++i) {
        if (!texture[i]) texture_text_static(renderer, &texture[i], static_speed_text[i], font, WHITE);
        if (texture[i]) {
            const float x = (i % 2) == 0 ? WIDTH * 0.369f : WIDTH - WIDTH * 0.369f;
            const float y_offset = HEIGHT * (0.5f * (i / 2));
            const SDL_FRect rect = calc_centered_rect(texture[i], x, HEIGHT * 0.1f + y_offset, HEIGHT * 0.075f);
            SDL_RenderTexture(renderer, texture[i], nullptr, &rect);
        }
    }
}

static void render_tires(const SDL_Color colors[4], unsigned short changed) {
    static SDL_Texture* texture[4]{nullptr};
    for (unsigned char i = 0; i < 4; ++i) {
        if (!texture[i]) texture_png(renderer, &texture[i], static_tire_path);
        if ((changed & (0b1 << i)) == (0b1 << i))
            SDL_SetTextureColorMod(texture[i], colors[i].r, colors[i].g, colors[i].b);
        if (texture[i]) {
            const float x = (i % 2) == 0 ? WIDTH * 0.025f : WIDTH - WIDTH * 0.025f - WIDTH * 0.15f;
            const float y_offset = HEIGHT * (0.5f * (i / 2));
            const SDL_FRect rect{x, HEIGHT * 0.025f + y_offset, WIDTH * 0.15f, HEIGHT * 0.45f};
            SDL_RenderTexture(renderer, texture[i], nullptr, &rect);
        }
    }
}

static void render_speed(char speed[4][sizeof(wheel_info_data::wheel_speed[0])], unsigned short changed) {
    static SDL_Texture* texture[4]{nullptr};
    for (unsigned char i = 0; i < 4; ++i) {
        if (!texture[i] or (changed & (0b10000 << i)) == (0b10000 << i))
            texture_text(renderer, &texture[i], speed[i], font, WHITE);
        if (texture[i]) {
            const float x = (i % 2) == 0 ? WIDTH * 0.369f : WIDTH - WIDTH * 0.369f;
            const float y_offset = HEIGHT * (0.5f * (i / 2));
            const SDL_FRect rect = calc_centered_rect(texture[i], x, HEIGHT * 0.2f + y_offset, HEIGHT * 0.075f);
            SDL_RenderTexture(renderer, texture[i], nullptr, &rect);
        }
    }
}

static void render_temperature(char temperature[4][sizeof(wheel_info_data::temperature[0])], unsigned short changed) {
    static SDL_Texture* texture[4]{nullptr};
    for (unsigned char i = 0; i < 4; ++i) {
        if (!texture[i] or (changed & (0b10000 << i)) == (0b10000 << i))
            texture_text(renderer, &texture[i], temperature[i], font, WHITE);
        if (texture[i]) {
            const float x = (i % 2) == 0 ? WIDTH * 0.369f : WIDTH - WIDTH * 0.369f;
            const float y_offset = HEIGHT * (0.5f * (i / 2));
            const SDL_FRect rect = calc_centered_rect(texture[i], x, HEIGHT * 0.3f + y_offset, HEIGHT * 0.075f);
            SDL_RenderTexture(renderer, texture[i], nullptr, &rect);
        }
    }
}

static void render_lateral_slip(float lateral[4], unsigned short changed) {
    static SDL_Texture* white_texture{nullptr};
    static SDL_Texture* orange_texture{nullptr};
    static float texture_width, texture_height;
    for (unsigned char i = 0; i < 4; ++i) {
        if (white_texture == nullptr) {
            texture_png(renderer, &white_texture, static_lateral_path);
            SDL_GetTextureSize(white_texture, &texture_width, &texture_height);
        }
        if (orange_texture == nullptr) {
            texture_png(renderer, &orange_texture, static_lateral_path);
            SDL_SetTextureColorMod(orange_texture, ORANGE.r, ORANGE.g, ORANGE.b);
        }
        if (white_texture != nullptr and orange_texture != nullptr and (changed & (0b10000 << i)) == (0b10000 << i)) {
            const float x = (i % 2) == 0 ? WIDTH * 0.369f : WIDTH - WIDTH * 0.369f;
            const float y = HEIGHT * 0.4;
            const float y_offset = HEIGHT * (0.5f * (i / 2));
            const float h = HEIGHT * 0.075;

            const SDL_FRect base_rect = calc_centered_rect(white_texture, x, y + y_offset, h);
            const float w = base_rect.w;

            // 1. Render the full background gauge in white

            // Get source texture dimensions
            float tex_w = 0.0f, tex_h = 0.0f;
            SDL_GetTextureSize(white_texture, &tex_w, &tex_h);

            // Calculate geometry proportions (assuming height = center circle diameter)
            const float circle_tex_x = (tex_w - tex_h) * 0.5f;
            const float bar_max_tex_w = circle_tex_x;
            const float bar_max_screen_w = (w - h) * 0.5f;

            // 2. Always draw the active orange center circle
            const SDL_FRect circle_src{circle_tex_x, 0.0f, tex_h, tex_h};
            const SDL_FRect circle_dst{x - h * 0.5f, base_rect.y, h, h};

            // Clamp input values between -1.0 (100% left) and 1.0 (100% right)
            // const float slip = std::clamp(lateral[i], -1.0f, 1.0f);

            SDL_RenderTexture(renderer, white_texture, nullptr, &base_rect);
            SDL_RenderTexture(renderer, orange_texture, &circle_src, &circle_dst);

            // 3. Render active directional slip bar from center outwards
            if (lateral[i] > 0.0f) {
                // Right slip fill
                const float tex_fill_w = bar_max_tex_w * lateral[i];
                const float screen_fill_w = bar_max_screen_w * lateral[i];

                const SDL_FRect right_src{(tex_w + tex_h) * 0.5f, 0.0f, tex_fill_w, tex_h};
                const SDL_FRect right_dst{x + h * 0.5f, base_rect.y, screen_fill_w, h};

                SDL_RenderTexture(renderer, orange_texture, &right_src, &right_dst);
            } else if (lateral[i] < 0.0f) {
                // Left slip fill
                const float abs_slip = -lateral[i];
                const float tex_fill_w = bar_max_tex_w * abs_slip;
                const float screen_fill_w = bar_max_screen_w * abs_slip;

                const SDL_FRect left_src{bar_max_tex_w - tex_fill_w, 0.0f, tex_fill_w, tex_h};
                const SDL_FRect left_dst{(x - h * 0.5f) - screen_fill_w, base_rect.y, screen_fill_w, h};

                SDL_RenderTexture(renderer, orange_texture, &left_src, &left_dst);
            }
        }
    }
}

static void render_suspension(float travel[4], unsigned short changed) {
    static SDL_Texture* white_texture{nullptr};
    static SDL_Texture* orange_texture{nullptr};
    static float texture_width, texture_height;
    for (unsigned char i = 0; i < 4; ++i) {
        if (white_texture == nullptr) {
            texture_png(renderer, &white_texture, static_suspension_path);
            SDL_GetTextureSize(white_texture, &texture_width, &texture_height);
        }
        if (orange_texture == nullptr) {
            texture_png(renderer, &orange_texture, static_suspension_path);
            SDL_SetTextureColorMod(orange_texture, ORANGE.r, ORANGE.g, ORANGE.b);
        }
        if (white_texture != nullptr and orange_texture != nullptr and (changed & (0b10000 << i)) == (0b10000 << i)) {
            const float x = (i % 2) == 0 ? WIDTH * 0.19f : WIDTH - WIDTH * 0.19f - WIDTH * 0.075f;
            const float y_offset = HEIGHT * (0.5f * (i / 2));

            const SDL_FRect base_rect{x, HEIGHT * 0.025f, WIDTH * 0.075f, HEIGHT * 0.45f};
            const SDL_FRect white_source_rect{0.f, 0.f, texture_width, texture_height * (1 - travel[i])};
            const SDL_FRect orange_source_rect{0.f, texture_height * (1 - travel[i]), texture_width,
                                               texture_height * travel[i]};
            const SDL_FRect white_destination_rect{base_rect.x, base_rect.y + y_offset, base_rect.w,
                                                   base_rect.h * (1 - travel[i])};
            const SDL_FRect orange_destination_rect{base_rect.x, base_rect.y + base_rect.h * (1 - travel[i]) + y_offset,
                                                    base_rect.w, base_rect.h * travel[i]};
            SDL_RenderTexture(renderer, white_texture, &white_source_rect, &white_destination_rect);
            SDL_RenderTexture(renderer, orange_texture, &orange_source_rect, &orange_destination_rect);
        }
    }
}

void wheel_info_t::render() {
    wheel_info_data data_copy;

    {
        std::lock_guard<std::mutex> lock(*mutex);
        if (data.new_data == 0 or data.is_paused) return;
        data_copy = data;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    render_static_speed_text();

    render_tires(data_copy.slipping, data_copy.new_data);
    render_temperature(data_copy.temperature, data_copy.new_data);
    render_speed(data_copy.wheel_speed, data_copy.new_data);
    render_lateral_slip(data_copy.lateral_slip, data_copy.new_data);
    render_suspension(data_copy.suspension, data_copy.new_data);

    SDL_RenderPresent(renderer);
}

void wheel_info_t::close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
}
