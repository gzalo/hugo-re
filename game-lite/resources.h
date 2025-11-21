#include "render_sdl.h"

#ifndef RESOURCES_H
#define RESOURCES_H

typedef struct {
    Texture** frames;
    int frame_count;
    int* sync_data;
    int sync_count;
} Animation;

Texture* animation_get_sync_frame(Animation animation, int frame_index);
Texture* animation_get_frame(Animation animation, int frame_index);

void init_textures(const char *data_dir);
void init_audio(const char *data_dir);

#endif