#ifndef TEXTURE_HANDLER_HPP
#define TEXTURE_HANDLER_HPP

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_pixels.h>
#include <vector>
#include <cstdio>
#include <stdlib.h>
#include <iostream>
#include <algorithm>

extern std::vector<SDL_Texture*> registered_textures;



void texture_text(SDL_Renderer*, SDL_Texture**, const char*, TTF_Font*, const SDL_Color&);

void texture_text_static(SDL_Renderer*, SDL_Texture**, const char*, TTF_Font*, const SDL_Color&) ;

void texture_png(SDL_Renderer*, SDL_Texture**,const char*);
void texture_png_static(SDL_Renderer*, SDL_Texture**,const char*);

void destroy_registered_textures();

#endif