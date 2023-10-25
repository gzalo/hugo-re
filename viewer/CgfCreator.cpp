#include "CgfCreator.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void CgfCreator::loadFrame(const char *filename, int i, int i1, int i2, int i3) {
    int w, h, ch;
    uint32_t *data = (uint32_t*) stbi_load(filename, &w, &h, &ch, 4);
    uint8_t *palData = new uint8_t[w*h];
    for(int i=0;i<w*h;i++){
        palData[i++] = graphics->getCollor(data[i++]);
    }
    free(data);
    totalFrames++;
}
CgfCreator::CgfCreator(Graphics *graphics) {
    this->graphics = graphics;
    this->totalFrames = 0;
}

struct CGFHeader {
    uint32_t magic;
    uint32_t flags;
    uint32_t totalFrames;
    uint32_t frameMetadataSize;
    uint32_t framePayloadSize;
    uint32_t unk1;
    uint32_t unk2;
};

struct CGFFrameMeta {
    uint32_t xOffset;
    uint32_t yOffset;
    uint32_t width;
    uint32_t height;
    uint32_t unk3;
    uint32_t payloadOffset;
};


void CgfCreator::save(const char *filename) {
    CGFHeader header = (CGFHeader){
        .magic = 0x46464743,
        .flags = 1,
        .totalFrames = (uint32_t) totalFrames,
        .frameMetadataSize = (uint32_t) sizeof(CGFFrameMeta)*totalFrames,
        .framePayloadSize = 5,
        .unk1 = 0,
         .unk2 = 0
    };
}

/*
 * for(int i=0;i<len;i++){
        uint8_t v = fileData[idx+i];
        i++;

        if(v == 0){
            uint8_t off = fileData[idx+i];
            if(off == 0){
                return;
            }
            buffer += off;
        } else if(v == 3){
            uint8_t count = fileData[idx+i];

            for(int j=0;j<count;j++) {
                i++;
                *(buffer++) = graphics->getPalleteCollor(fileData[idx+i]);
            }
        } else if(v == 4){
            uint8_t count = fileData[idx+i];

            i++;
            uint8_t value = fileData[idx+i];

            for(int j=0;j<count;j++) {
                *(buffer++) = graphics->getPalleteCollor(value);
            }
        } else {
            printf("Unk%x\n", v);
        }
    }
 */
