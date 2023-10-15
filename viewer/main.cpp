#include <SDL2/SDL.h>
#include <ctime>
#include <cstdio>

#define WINDOW_WIDTH (640*2)
#define WINDOW_HEIGHT (480*2)

#define RENDER_WIDTH (640)
#define RENDER_HEIGHT (480)

Uint32 pixels[RENDER_WIDTH * RENDER_HEIGHT] = {0};

const int bufferLen = 8*1024*1024;
uint8_t data[bufferLen] = {0};
uint32_t pallette[256] = {0};

uint8_t palleteFileData[0x326] = {0};

uint16_t framesMax = 75;
uint32_t frameOffset[75] = {0};
int currentFrame = 0;
int dataLen = 0;
int tileDataOffset = 0;

int qualityFactor = 1; // 1 -> regular quality
// 2 -> x half width
// 4 -> xy half

void loadData(){
    FILE *input = fopen("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\key\\key1.cbr", "rb");
    dataLen = fread(data, 1, bufferLen, input);
    fclose(input);
}

void loadOffsets(){
    FILE *inputOffsets = fopen("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\skate\\vulcano.off", "rb");
    fread(frameOffset, 4, framesMax, inputOffsets);
    fclose(inputOffsets);
}

void loadPalette(){
    FILE *inputOffsets = fopen("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\menu\\vgapal.raw", "rb");
    //FILE *inputOffsets = fopen("C:\\Users\\Gzalo\\Downloads\\quickbms\\4hugo\\key\\stikkein.til", "rb");
    fread(palleteFileData, 1, 0x326, inputOffsets);
    fclose(inputOffsets);

     for(int i=0;i<256;i++) {
           int offset = 0x20 + i*3;
            //pallette[i] = palleteFileData[offset] | (palleteFileData[offset+1]<<8) | (palleteFileData[offset+2]<<16) | 0xFF000000;
            pallette[i] = palleteFileData[offset+2] | (palleteFileData[offset+1]<<8) | (palleteFileData[offset]<<16) | 0xFF000000;
        }
}

int8_t pbrGetHighContrib(uint8_t x){
    uint8_t a = x >> 4;
    if (a & 0x08) {
        return -((~a & 0x0F) + 1);
    } else {
        return a;
    }
}

int8_t pbrGetLowContrib(uint8_t x){
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
    /*if(val == 0){ //Transparent
        return;
    }*/
    pixels[y * RENDER_WIDTH + x] = pallette[val];
}

void pbrUnpackbits(uint32_t idx, int largo, int startingOffset, int y) {
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
                    int8_t c0 = pbrGetHighContrib(current);
                    int8_t c1 = pbrGetLowContrib(current);
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

void pbrParsePart(uint32_t start, uint32_t len, int *y) {
    printf("Processing part at %x len %d\n", start, len);

    uint16_t *p = (uint16_t*)(data+start);

    printf("X:%d Y:%d width:%d height:%d\n", p[0], p[1], p[2], p[3]);

    uint32_t idx = start+8;

    while(idx < start+len) {
        uint16_t offset = *(uint16_t*)(data+idx);
        uint16_t len = *(uint16_t*)(data+idx+2);
        idx+=4;
        pbrUnpackbits(idx, len, offset, (*y)++);
        idx+=len;
    }
}

void cbrUnpackbits(uint32_t idx, int largo, int startingOffset, int x, int y) {
    printf("%x\n", startingOffset);
    for (int i = 0; i < largo; i++) {
        printf("%02x ", data[i + idx]);
    }
    printf("\n");

    for(int i=0;i<largo;i++){
        setPixelPallete(i+startingOffset+x, y, data[i+idx]);
    }
}

void cbrParsePart(uint32_t start, uint32_t len) {
    printf("Processing part at %x len %d\n", start, len);

    uint32_t *p = (uint32_t*)(data+start);

    printf("width:%d height:%d X:%d Y:%d\n", p[0], p[1], p[2], p[3]);

    uint32_t idx = start+16;
    int y = p[3];
    int x = p[2];

    while(idx < start+len) {
        uint16_t offset = *(uint16_t*)(data+idx);
        uint16_t len = *(uint16_t*)(data+idx+2);
        idx+=4;
        cbrUnpackbits(idx, len, offset, x, y++);
        idx+=len;
    }
}

void drawTile(int tileId, int xOff, int yOff, int tileWidth, int tileHeight) {

    int offset = tileDataOffset + tileId*tileHeight*tileWidth;
    for (int y = 0; y < tileHeight; y++)
        for (int x = 0; x < tileWidth; x++) {
            uint8_t color = data[offset++];
            setPixelPallete(x + xOff, y + yOff, color);
        }
}

void renderTil(int offset, int tileWidth, int tileHeight, int totalWidthTiles, int totalHeightTiles) {
    if(qualityFactor == 2){
        tileWidth/=2;
    }
    if(qualityFactor == 4){
        tileWidth/=2;
        tileHeight/=2;
    }
    for(int l=0;l<totalWidthTiles*totalHeightTiles;l++){
        uint16_t tileId = data[offset] + data[offset+1]*256;
        offset+=2;

        int xOff = (l%totalWidthTiles)*tileWidth;
        int yOff = (l/totalWidthTiles)*tileHeight;

        drawTile(tileId, xOff, yOff, tileWidth, tileHeight);
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

    loadData();
    loadOffsets();
    loadPalette();

    freopen("out.txt", "w", stdout);

    // TIL
    /*int totalWidth = (data[8] | (data[9] <<8));
    int totalHeight = (data[10] | (data[11] <<8));

    int totalWidthTiles = data[0x15];
    int totalHeightTiles = data[0x17];

    int tileWidth = totalWidth/totalWidthTiles;
    int tileHeight = totalHeight/totalHeightTiles;

    int base = 0x320;

    framesMax = (data[6] | (data[7] <<8))+2;
    tileDataOffset = framesMax*totalWidthTiles*totalHeightTiles*2+base;

    int delta = totalWidthTiles*totalHeightTiles*2;*/

    /* PBR */
    int y = 0;
    uint32_t offsetToOffsets = *(uint32_t*)data;
    uint32_t totalPartCount = *(uint32_t*)(data+0xC);
    uint32_t *offsets = (uint32_t*)(data+offsetToOffsets);

    for(int i=0;i<totalPartCount;i++){
        uint32_t end = i == totalPartCount - 1 ? dataLen-4*totalPartCount : offsets[i+1];
        pbrParsePart(offsets[i], end - offsets[i], &y);
    }

    /* CBR */
   /* uint32_t offsetToOffsets = *(uint32_t*)data;
    uint32_t totalPartCount = (dataLen-offsetToOffsets)/4;
    uint32_t *offsets = (uint32_t*)(data+offsetToOffsets);
    framesMax = totalPartCount;
*/


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

        /* TIL */
       // renderTil(base+currentFrame*delta, tileWidth, tileHeight, totalWidthTiles, totalHeightTiles);


        /* CBR */
       /* uint32_t end = currentFrame == totalPartCount - 1 ? dataLen-4*totalPartCount : offsets[currentFrame+1];
        cbrParsePart(offsets[currentFrame], end - offsets[currentFrame]);*/


       // BRS
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
