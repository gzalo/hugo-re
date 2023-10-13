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

uint8_t palleteFileData[0x326] = {0};

const uint16_t framesMax = 75;
uint32_t frameOffset[framesMax] = {0};
int currentFrame = 0;
int dataLen = 0;

void loadData(){
    FILE *input = fopen("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\ice\\hopv2.cbr", "rb");
    dataLen = fread(data, 1, bufferLen, input);
    fclose(input);
}

void loadOffsets(){
    FILE *inputOffsets = fopen("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\lumber\\sak-gren.ofs", "rb");
    fread(frameOffset, 4, framesMax, inputOffsets);
    fclose(inputOffsets);
}

void loadPalette(){
    FILE *inputOffsets = fopen("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\scuba\\pcpal.raw", "rb");
    fread(palleteFileData, 1, 0x326, inputOffsets);
    fclose(inputOffsets);

     for(int i=0;i<256;i++) {
           int offset = 0x20 + i*3;
            //pallette[i] = palleteFileData[offset] | (palleteFileData[offset+1]<<8) | (palleteFileData[offset+2]<<16) | 0xFF000000;
            pallette[i] = palleteFileData[offset+2] | (palleteFileData[offset+1]<<8) | (palleteFileData[offset]<<16) | 0xFF000000;
        }
}

int8_t getHighContrib(uint8_t x){
    uint8_t a = x >> 4;
    if (a & 0x08) {
        return -((~a & 0x0F) + 1);
    } else {
        return a;
    }
}

int8_t getLowContrib(uint8_t x){
    uint8_t a = x & 0x0F;
    if (a & 0x08) {
        return -((~a & 0x0F) + 1);
    } else {
        return a;
    }
}

void setPixelPallete(int x, int y, uint8_t val){
    if(x>=RENDER_WIDTH || y >= RENDER_HEIGHT){
        return;
    }
    pixels[y * RENDER_WIDTH + x] = pallette[val];
}

void unpackbits(uint32_t idx, int largo, int startingOffset, int y) {
    printf("\n");
    for(int i=0;i<largo;i++){
        printf("%02x ", data[i+idx]);
    }
    printf("\n");

    for(int i=0;i<startingOffset;i++){
        printf("[  ] ");
    }
    int x = startingOffset;

    for(int i=0;i<largo;i++){
        int8_t id = data[idx+i];
        uint8_t uid = data[idx+i];

        if(id < 0){
            if((uid & 0b11100000) == 0b10100000){
                printf("?M%x?", uid);

                int len = (~uid & 0xF)+2;
                if((uid & 0b11110000) == 0b10100000) len+=16;
                printf("L:%d ", len);
                //AF
                uint8_t v0 = 0;
                uint8_t v1 = 0;
                for (int j = 0; j < len; j++) {
                    i++;
                    uint8_t current = data[idx + i];
                    int8_t c0 = getHighContrib(current);
                    int8_t c1 = getLowContrib(current);
                    v0 = j == 0 ? current : v1+c0;
                    v1 = j == 0 ? current : v0+c1;
                    //printf("%x V0 %x V1 %x\n", current, v0, v1);
                    printf("[%02x] ", v0);
                    printf("[%02x] ", v1);
                    if(j != 0) {
                        setPixelPallete(x,y, v0);
                        x++;
                    }
                    setPixelPallete(x,y, v1);
                    x++;
                }
            } else {
                printf("?N%x?", uid);
                for (int j = 0; j < -id; j++) {
                    i++;
                    printf("[%02x] ", data[idx + i]);
                    setPixelPallete(x,y, data[idx+i]);
                    x++;
                }
            }
        } else {
            //printf("(R%d) ", id);
            uint8_t repeatVal = data[idx+i+1];
            for(int j=0;j<id;j++){
                printf("[%02x] ", repeatVal);
                setPixelPallete(x,y, repeatVal);
                x++;
            }
            i++;
        }

    }
    printf("\n");
}

void parsePart(uint32_t start, uint32_t len, int *y) {
    printf("Processing part at %x len %d\n", start, len);

    uint16_t *p = (uint16_t*)(data+start);

    printf("X:%d Y:%d width:%d height:%d\n", p[0], p[1], p[2], p[3], p[4]);

    uint32_t idx = start+8;

    while(idx < start+len) {
        uint16_t offset = *(uint16_t*)(data+idx);
        uint16_t len = *(uint16_t*)(data+idx+2);
        idx+=4;
        unpackbits(idx, len, offset, (*y)++);
        idx+=len;
    }
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

    freopen("out.txt", "w", stdout);

    uint32_t offsetToOffsets = *(uint32_t*)data;
    uint32_t totalPartCount = *(uint32_t*)(data+0xC);
    uint32_t *offsets = (uint32_t*)(data+offsetToOffsets);
    int y = 0;

    for(int i=0;i<totalPartCount;i++){
        uint32_t end = i == totalPartCount - 1 ? dataLen-4*totalPartCount : offsets[i+1];
        parsePart(offsets[i], end-offsets[i], &y);
    }

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

        /*for(int i=0;i<RENDER_WIDTH*RENDER_HEIGHT;i++)
            pixels[i] = 0xFFFFFFFF;*/

        /*int startingOffset = frameOffset[currentFrame];

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
        }*/



        /*for(int y=0;y<1000;y++) {
            for(int x=0;x<wmod;x++) {
                int i = x + y*wmod + startingOffset;
                if(data[i] != 0 && x+y*640 < 640*480)
                    pixels[x+y*640] = pallette[data[i]];
            }
        }*/

        SDL_UpdateTexture(texture, NULL, pixels, RENDER_WIDTH * sizeof(Uint32));
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        char title[256] = "";
        sprintf(title, "Frame: %d, %x", currentFrame, 0);
        SDL_SetWindowTitle(window, title);

        SDL_Delay(16);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
