#include "Graphics.h"
#include <ctime>

Graphics::Graphics() {
    pixels = new uint32_t[renderWidth*renderHeight];
}

void Graphics::setPixelPallete(int x, int y, uint8_t val){
    if(x>=renderWidth || y >= renderHeight){
        return;
    }
    if(zeroIsTransparent && val == 0){
        return;
    }
    pixels[y * renderWidth + x] = palette[val];
}

void Graphics::setPalleteCollor(int idx, uint32_t color) {
    if(idx >= 256){
        return;
    }
    //printf("SetPalColor: %x => %x\n", idx, color);
    palette[idx] = color;
}

void Graphics::init() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(windowWidth, windowHeight, 0, &window, &renderer);
    texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, renderWidth, renderHeight);
}

void Graphics::deinit() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Graphics::render(const string &title) {
    SDL_UpdateTexture(texture, nullptr, pixels, renderWidth * sizeof(Uint32));
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    SDL_SetWindowTitle(window, title.c_str());
    SDL_Delay(16);
}

void Graphics::clear() {
    for(int i=0;i<renderWidth*renderHeight;i++)
        pixels[i] = 0xFFFFFFFF;
}
