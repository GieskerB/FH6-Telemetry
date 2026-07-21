#include "../../include/util/texture_handler.hpp"

std::vector<SDL_Texture*> registered_textures;

SDL_FRect calc_centered_rect(SDL_Texture* texture, float center_x, float center_y, float target_height) {
    float w, h;
    SDL_GetTextureSize(texture, &w, &h);

    const float ratio = w / h;
    const float final_width = target_height * ratio;

    return {center_x - (final_width / 2.f), center_y - (target_height / 2.f), final_width, target_height};
}

SDL_FRect calc_left_rect(SDL_Texture* texture, float left_x, float left_y, float target_height) {
    float w, h;
    SDL_GetTextureSize(texture, &w, &h);

    const float ratio = w / h;
    const float final_width = target_height * ratio;

    return {left_x, left_y, final_width, target_height};
}

void texture_text(SDL_Renderer* renderer, SDL_Texture** texture, const char* text, TTF_Font* font,
                  const SDL_Color& color) {
    SDL_Surface* surf = TTF_RenderText_Blended(font, text, 0, color);
    if (!surf) return;

    if (!*texture) {
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STREAMING, surf->w, surf->h);
        if (!*texture) return;
        registered_textures.push_back(*texture);
    }

    const int tex_w = (*texture)->w;
    const int tex_h = (*texture)->h;

    if (tex_w < surf->w or tex_h < surf->h) {
        int max_width = tex_w > surf->w ? tex_w : surf->w;
        int max_height = tex_h > surf->h ? tex_h : surf->h;
        // Destroy old texture.
        auto it = std::find(registered_textures.begin(), registered_textures.end(), *texture);
        if (it != registered_textures.end()) registered_textures.erase(it);
        SDL_DestroyTexture(*texture);
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STREAMING, max_width, max_height);
        if (!*texture) return;
        registered_textures.push_back(*texture);
    }

    SDL_Rect rect{0, 0, surf->w, surf->h};
    SDL_UpdateTexture(*texture, &rect, surf->pixels, surf->pitch);
    SDL_DestroySurface(surf);
}

void texture_text_static(SDL_Renderer* renderer, SDL_Texture** texture, const char* text, TTF_Font* font,
                         const SDL_Color& color) {
    SDL_Surface* surf = TTF_RenderText_Blended(font, text, 0, color);
    if (!surf) return;
    if (!*texture) {
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STATIC, surf->w, surf->h);
        if (!*texture) {
            SDL_DestroySurface(surf);
            return;
        }
        registered_textures.push_back(*texture);
    }

    SDL_UpdateTexture(*texture, NULL, surf->pixels, surf->pitch);
    SDL_DestroySurface(surf);
}

void texture_png(SDL_Renderer* renderer, SDL_Texture** texture, const char* file_name) {
    SDL_Surface* surf = SDL_LoadPNG(file_name);
    if (!surf) return;

    if (!*texture) {
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STREAMING, surf->w, surf->h);
        if (!*texture) return;
        registered_textures.push_back(*texture);
    }

    SDL_UpdateTexture(*texture, NULL, surf->pixels, surf->pitch);
    SDL_DestroySurface(surf);
}

void texture_png_static(SDL_Renderer* renderer, SDL_Texture** texture, const char* file_name) {
    SDL_Surface* surf = SDL_LoadPNG(file_name);
    if (!surf) return;

    if (!*texture) {
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STATIC, surf->w, surf->h);
        if (!*texture) return;
        registered_textures.push_back(*texture);
    }

    SDL_UpdateTexture(*texture, NULL, surf->pixels, surf->pitch);
    SDL_DestroySurface(surf);
}

void destroy_registered_textures() {
    for (const auto& texture : registered_textures) {
        SDL_DestroyTexture(texture);
    }
}
