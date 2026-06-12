#ifndef TEXTURE_HANDLER_HPP
#define TEXTURE_HANDLER_HPP

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_pixels.h>
#include <vector>

extern std::vector<SDL_Texture*> registered_textures;

template<int BUFF_SIZE, typename NUM_TYPE>
void texture_text(SDL_Renderer* renderer, SDL_Texture** texture, const char format_string[], NUM_TYPE value, TTF_Font* font, const SDL_Color &color) {
    char buffer[BUFF_SIZE]{0};
    SDL_snprintf(buffer, sizeof(buffer), format_string, value);
    SDL_Surface* surf = TTF_RenderText_Blended(font, buffer, 0, color);
    
    if (!*texture) {
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STREAMING, surf->w, surf->h);
        registered_textures.push_back(*texture);
    }

    if (surf) {
        SDL_UpdateTexture(*texture, NULL, surf->pixels, surf->pitch);
        SDL_DestroySurface(surf);
    }
}

template<int BUFF_SIZE, typename NUM_TYPE>
void texture_text_static(SDL_Renderer* renderer, SDL_Texture** texture, const char format_string[], NUM_TYPE value, TTF_Font* font, const SDL_Color &color) {
    char buffer[BUFF_SIZE]{0};
    SDL_snprintf(buffer, sizeof(buffer), format_string, value);
    SDL_Surface* surf = TTF_RenderText_Blended(font, buffer, 0, color);
    
    if (!*texture) {
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STATIC, surf->w, surf->h);
        registered_textures.push_back(*texture);
    }
    
    if (surf) {
        SDL_UpdateTexture(*texture, NULL, surf->pixels, surf->pitch);
        SDL_DestroySurface(surf);
    }
}

void texture_png(SDL_Renderer* renderer, SDL_Texture** texture,const char file_name[]);
void texture_png_static(SDL_Renderer* renderer, SDL_Texture** texture,const char file_name[]);

void destroy_registered_textures();

#endif