#include "../../include/util/texture_handler.hpp"

std::vector<SDL_Texture*> registered_textures;

static void check_texture_size(SDL_Renderer* renderer,SDL_Texture** texture,SDL_Surface* surf) {
    if ((*texture)->w < surf->w or (*texture)->h < surf->h) {
        int max_width = (*texture)->w > surf->w ? (*texture)->w : surf->w;
        int max_height = (*texture)->h > surf->h ? (*texture)->h : surf->h;
        // Destroy old texture.
        std::remove(registered_textures.begin(), registered_textures.end(), *texture);
        SDL_DestroyTexture(*texture);
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STREAMING, max_width, max_height);
        if(*texture == nullptr) {
            perror(SDL_GetError());
            exit(EXIT_FAILURE);
        }
        registered_textures.push_back(*texture);
    }
}

void texture_text(SDL_Renderer* renderer, SDL_Texture** texture, const char* text, TTF_Font* font, const SDL_Color &color) {
    SDL_Surface* surf = TTF_RenderText_Blended(font, text, 0, color);

    if (!*texture) {
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STREAMING, surf->w, surf->h);
        if(*texture == nullptr) {
            perror(SDL_GetError());
            exit(EXIT_FAILURE);
        }
        registered_textures.push_back(*texture);
    }

    if (surf) {
        check_texture_size(renderer,texture, surf);
        SDL_UpdateTexture(*texture, NULL, surf->pixels, surf->pitch);
        SDL_DestroySurface(surf);
    }
}

void texture_text_static(SDL_Renderer* renderer, SDL_Texture** texture, const char* text, TTF_Font* font, const SDL_Color &color) {
    SDL_Surface* surf = TTF_RenderText_Blended(font, text, 0, color);
    
    if (!*texture) {
        *texture = SDL_CreateTexture(renderer, surf->format, SDL_TEXTUREACCESS_STATIC, surf->w, surf->h);
        if(*texture == nullptr) {
            perror(SDL_GetError());
            exit(EXIT_FAILURE);
        }
        registered_textures.push_back(*texture);
    }
    
    if (surf) {
        check_texture_size(renderer,texture, surf);

        SDL_UpdateTexture(*texture, NULL, surf->pixels, surf->pitch);
        SDL_DestroySurface(surf);
    }
}

void texture_png(SDL_Renderer* renderer, SDL_Texture** texture,const char* file_name) {
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

void texture_png_static(SDL_Renderer* renderer, SDL_Texture** texture,const char* file_name) {
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