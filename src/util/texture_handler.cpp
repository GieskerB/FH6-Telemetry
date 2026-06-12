# include "../../include/util/texture_handler.hpp"

void texture_png(SDL_Renderer* renderer, SDL_Texture** texture,const char file_name[]) {
    SDL_Surface* surf = SDL_LoadPNG(file_name);
    
    if (!*texture) {
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STREAMING, surf->w, surf->h);
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
    }
    
    if (surf) {
        SDL_UpdateTexture(*texture, NULL, surf->pixels, surf->pitch);
        SDL_DestroySurface(surf);
    }
}