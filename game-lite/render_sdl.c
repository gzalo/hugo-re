#include "render_sdl.h"
#include "config.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int query_texture_width(Texture *texture) {
    int w = 0;
    int h = 0;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    return w;
}

int get_time_ms() {
    return SDL_GetTicks();
}

void render_cleanup() {
    Mix_CloseAudio();
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    
    if (window) {
        SDL_DestroyWindow(window);
    }
    
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}

Texture* load_texture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Warning: Could not load image %s: %s\n", path, IMG_GetError());
        return NULL;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        printf("Warning: Could not create texture from %s: %s\n", path, SDL_GetError());
    }
    
    return texture;
}

void render(Texture *texture, int x, int y){
    if(texture){
        SDL_Rect dest_rect;
        dest_rect.x = x;
        dest_rect.y = y;
        SDL_QueryTexture(texture, NULL, NULL, &dest_rect.w, &dest_rect.h);
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
    }
}

void render_subtexture(Texture* texture, int sx, int sy, int w, int h, int dx, int dy){
    if(texture){
        SDL_Rect src_rect;
        src_rect.x = sx;
        src_rect.y = sy;
        src_rect.w = w;
        src_rect.h = h;

        SDL_Rect dest_rect;
        dest_rect.x = dx;
        dest_rect.y = dy;
        dest_rect.w = w;
        dest_rect.h = h;

        SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect);
    }
}

void play(Audio *audio){
    if(audio){
        Mix_PlayChannel(-1, audio, 0);
    }
}

bool render_init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Hugo",
                             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE,
                             SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    
    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    // Initialize SDL_mixer (optional)
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Warning: SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    Mix_AllocateChannels(16);
    
    return true;
}

void render_step() {
    SDL_RenderPresent(renderer);
    SDL_Delay(1000 / 30);
}

bool render_getevents(InputState* input_state) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return true;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                return true;
            } else if (event.key.keysym.sym == SDLK_3) {
                input_state->cave_rope_1_pressed = true;
            } else if (event.key.keysym.sym == SDLK_6) {
                input_state->cave_rope_2_pressed = true;
            } else if (event.key.keysym.sym == SDLK_9) {
                input_state->cave_rope_3_pressed = true;
            } else if (event.key.keysym.sym == SDLK_2 || event.key.keysym.sym == SDLK_UP) {
                input_state->key_up = true;
            } else if (event.key.keysym.sym == SDLK_8 || event.key.keysym.sym == SDLK_DOWN) {
                input_state->key_down = true;
            } else if (event.key.keysym.sym == SDLK_5) {
                input_state->key_start = true;
            }
        } else if (event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_3) {
                input_state->cave_rope_1_pressed = false;
            } else if (event.key.keysym.sym == SDLK_6) {
                input_state->cave_rope_2_pressed = false;
            } else if (event.key.keysym.sym == SDLK_9) {
                input_state->cave_rope_3_pressed = false;
            } else if (event.key.keysym.sym == SDLK_2 || event.key.keysym.sym == SDLK_UP) {
                input_state->key_up = false;
            } else if (event.key.keysym.sym == SDLK_8 || event.key.keysym.sym == SDLK_DOWN) {
                input_state->key_down = false;
            } else if (event.key.keysym.sym == SDLK_5) {
                input_state->key_start = false;
            }
        }
    }
    return false;
}