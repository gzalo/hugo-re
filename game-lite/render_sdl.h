// Rendering functions using SDL as a backend

#ifndef RENDER_SDL_H
#define RENDER_SDL_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "state.h"

// SDL-related typedefs
typedef SDL_Texture Texture;
typedef Mix_Chunk Audio;

int query_texture_width(Texture *texture);
Texture* load_texture(const char* path);
void render(Texture *texture, int x, int y);
void render_subtexture(Texture* texture, int sx, int sy, int w, int h, int dx, int dy);
void play(Audio *audio);

int get_time_ms();

bool render_init();
void render_cleanup();
void render_step();
bool render_getevents(InputState* input_state);

#endif