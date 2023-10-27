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

uint32_t Graphics::getPalleteCollor(uint8_t idx) {
    uint32_t argb_color = palette[idx];
    unsigned char alpha = (argb_color >> 24) & 0xFF;
    unsigned char red = (argb_color >> 16) & 0xFF;
    unsigned char green = (argb_color >> 8) & 0xFF;
    unsigned char blue = argb_color & 0xFF;
    return (alpha << 24) | (blue << 16) | (green << 8) | red;
}

uint8_t Graphics::getColor(uint32_t color) {
    if(color == 0) return 0xFF;
    unsigned char alpha = (color >> 24) & 0xFF;
    unsigned char red = (color >> 16) & 0xFF;
    unsigned char green = (color >> 8) & 0xFF;
    unsigned char blue = color & 0xFF;
    uint32_t lookup = (alpha << 24) | (blue << 16) | (green << 8) | red;
    for(int i=0;i<256;i++){
        if(palette[i] == lookup){
            return i;
        }
    }
    printf("CANT FIND COLOR for %x\n", color);
    return 0xFF;
}
