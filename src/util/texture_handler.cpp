#include "../../include/util/texture_handler.hpp"

std::vector<SDL_Texture*> registered_textures;

void texture_png(SDL_Renderer* renderer, SDL_Texture** texture,const char file_name[]) {
    SDL_Surface* surf = SDL_LoadPNG(file_name);
    
    if (!*texture) {
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STREAMING, surf->w, surf->h);
        registered_textures.push_back(*texture);
    }

    if (surf) {
        SDL_UpdateTexture(*texture, NULL, surf->pixels, surf->pitch);
        SDL_DestroySurface(surf);
    }
}

void texture_png_static(SDL_Renderer* renderer, SDL_Texture** texture,const char file_name[]) {
    SDL_Surface* surf = SDL_LoadPNG(file_name);
    
    if (!*texture) {
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STATIC, surf->w, surf->h);
        registered_textures.push_back(*texture);
    }

    if (surf) {
        SDL_UpdateTexture(*texture, NULL, surf->pixels, surf->pitch);
        SDL_DestroySurface(surf);
    }
}

void destroy_registered_textures() {
    for (const auto& texture: registered_textures) {
        SDL_DestroyTexture(texture);
    }
}