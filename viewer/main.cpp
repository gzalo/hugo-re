#include <SDL2/SDL.h>
#include <ctime>
#include <cstdio>

#define WINDOW_WIDTH (640*2)
#define WINDOW_HEIGHT (480*2)

#define RENDER_WIDTH (640)
#define RENDER_HEIGHT (480)

Uint32 pixels[RENDER_WIDTH * RENDER_HEIGHT] = {0};

const int bufferLen = 3*1024*1024;
uint8_t data[bufferLen] = {0};
uint32_t pallette[256] = {0};

const uint16_t framesMax = 75;
uint32_t frameOffset[framesMax] = {0};
int currentFrame = 0;

void loadData(){
    FILE *input = fopen("data/evilwood.brs", "rb");
    fread(data, bufferLen, 1, input);
    fclose(input);
}

void loadOffsets(){
    FILE *inputOffsets = fopen("data/evilwood.ofs", "rb");
    fread(frameOffset, 4, framesMax, inputOffsets);
    fclose(inputOffsets);
}

void loadPalette(){
    //    for(int i=0;i<256;i++) {
    //        int offset = 0x20 + i*3;
    //        pallette[i] = data[offset] | (data[offset+1]<<8) | (data[offset+2]<<16) | 0xFF000000;
    //    }
}

int main(int argc, char **args){

    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    srand(time(nullptr));

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);
    SDL_Texture *texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, RENDER_WIDTH, RENDER_HEIGHT);

    for(int i=0;i<RENDER_WIDTH*RENDER_HEIGHT;i++)
        pixels[i] = 0xFFFFFFFF;

    loadData();
    loadOffsets();
    loadPalette();

    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&event)){
          if(event.type == SDL_QUIT){
              quit = true;
          }
            if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.sym == SDLK_LEFT)
                    currentFrame = (currentFrame-1+framesMax)%framesMax;
                if(event.key.keysym.sym == SDLK_RIGHT)
                    currentFrame = (currentFrame+1)%framesMax;

            }
        }

        for(int i=0;i<RENDER_WIDTH*RENDER_HEIGHT;i++)
            pixels[i] = 0xFFFFFFFF;

        int startingOffset = frameOffset[currentFrame];

        uint32_t *dwords = (uint32_t*) (&data[startingOffset]);
        uint32_t w = dwords[0];
        uint32_t h = dwords[1];
        uint32_t p = dwords[2];
        uint32_t q = dwords[3];

        for(int y=0;y<h;y++) {
            for(int x=0;x<w;x++) {
                int i = x + y*w + startingOffset + 16;
                if(data[i] != 0)
                pixels[(x+p)+(y+q)*640] = data[i] * 0xF1323;
            }
        }

//        int startingOffset = 0x62D48;
//        int len = 0x16D64-0x300;
//        int n = 0;
//        for(int i=0;i<len;i+=2){
//            uint8_t repeatcount = data[startingOffset+i];
//            uint8_t repeatval = data[startingOffset+i+1];
//
//            for(int x=0;x<repeatcount;x++)
//                if(n < 640*480)
//                    pixels[n++] = pallette[repeatval];
//        }

//        for(int y=0;y<1000;y++) {
//            for(int x=0;x<wmod;x++) {
//                int i = x + y*wmod + startingOffset;
//                if(data[i] != 0 && x+y*640 < 640*480)
//                    pixels[x+y*640] = pallette[data[i]];
//            }
//        }

        SDL_UpdateTexture(texture, NULL, pixels, RENDER_WIDTH * sizeof(Uint32));
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        char title[256] = "";
        sprintf(title, "Frame: %d, %x", currentFrame, startingOffset);
        SDL_SetWindowTitle(window, title);

        SDL_Delay(16);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
