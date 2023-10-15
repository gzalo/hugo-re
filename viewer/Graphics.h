#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <cstdint>
#include <string>
#include <SDL2/SDL.h>
using namespace std;

class Graphics {
    const int renderWidth = 320;
    const int renderHeight = 256;
    const int scaleFactor = 2;
    const int windowWidth = renderWidth*scaleFactor;
    const int windowHeight = renderHeight*scaleFactor;

    uint32_t *pixels;
    uint32_t palette[256] = {0};
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Texture *texture;

    public:
        bool zeroIsTransparent = false;
        Graphics();
        void setPixelPallete(int x, int y, uint8_t val);
        void setPalleteCollor(int idx, uint32_t color);

    void init();

    void deinit();

    void render(const string &title);

    void clear();
};


#endif
