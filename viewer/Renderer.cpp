#include "Renderer.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <fstream>
#include <iostream>

int8_t Renderer::pbrGetHighContrib(uint8_t x){
    uint8_t a = x >> 4;
    if (a & 0x08) {
        return -((~a & 0x0F) + 1);
    } else {
        return a;
    }
}

int8_t Renderer::pbrGetLowContrib(uint8_t x){
    uint8_t a = x & 0x0F;
    if (a & 0x08) {
        return -((~a & 0x0F) + 1);
    } else {
        return a;
    }
}

void Renderer::pbrUnpackbits(uint32_t idx, int largo, int startingOffset, int y) {
    int x = startingOffset;

    for(int i=0;i<largo;i++){
        int8_t id = fileData[idx + i];
        uint8_t uid = fileData[idx + i];

        if(id < 0){
            if((uid & 0b11100000) == 0b10100000){

                int len = (~uid & 0xF)+2;
                if((uid & 0b11110000) == 0b10100000) len+=16;
                //AF
                uint8_t v0 = 0;
                uint8_t v1 = 0;
                for (int j = 0; j < len; j++) {
                    i++;
                    uint8_t current = fileData[idx + i];
                    int8_t c0 = pbrGetHighContrib(current);
                    int8_t c1 = pbrGetLowContrib(current);
                    v0 = j == 0 ? current : v1+c0;
                    v1 = j == 0 ? current : v0+c1;
                    if(j != 0) {
                        graphics->setPixelPallete(x,y, v0);
                        x++;
                    }
                    graphics->setPixelPallete(x,y, v1);
                    x++;
                }
            } else {
                for (int j = 0; j < -id; j++) {
                    i++;
                    graphics->setPixelPallete(x, y, fileData[idx + i]);
                    x++;
                }
            }
        } else {
            uint8_t repeatVal = fileData[idx + i + 1];
            for(int j=0;j<id;j++){
                graphics->setPixelPallete(x,y, repeatVal);
                x++;
            }
            i++;
        }

    }
}

void Renderer::pbrParsePart(uint32_t start, uint32_t len) {
    int x=*(uint16_t*)(fileData + start);
    int y=*(uint16_t*)(fileData + start+2);
    uint32_t idx = start+8;

    while(idx < start+len) {
        uint16_t offset = *(uint16_t*)(fileData + idx)+x;
        uint16_t sectionLen = *(uint16_t*)(fileData + idx + 2);
        idx+=4;
        pbrUnpackbits(idx, sectionLen, offset, y++);
        idx+=sectionLen;
    }
}

void Renderer::cbrUnpackbits(uint32_t idx, int largo, int startingOffset, int x, int y) {
    for(int i=0;i<largo;i++){
        graphics->setPixelPallete(i+startingOffset+x, y, fileData[i + idx]);
    }
}

void Renderer::cbrParsePart(uint32_t start, uint32_t len) {
    uint32_t *p = (uint32_t*)(fileData + start);

    uint32_t idx = start+16;
    int y = p[3];
    int x = p[2];

    while(idx < start+len) {
        uint16_t offset = *(uint16_t*)(fileData + idx);
        uint16_t sectionLen = *(uint16_t*)(fileData + idx + 2);
        idx+=4;
        cbrUnpackbits(idx, sectionLen, offset, x, y++);
        idx+=sectionLen;
    }
}

void Renderer::tilDrawTile(int tileId, int tileRealWidth, int tileRealHeight, int xOff, int yOff) {
    int offset = tileDataOffset + tileId*tileRealHeight*tileRealWidth;
    for (int y = 0; y < tileRealHeight; y++)
        for (int x = 0; x < tileRealWidth; x++) {
            uint8_t color = fileData[offset++];
            graphics->setPixelPallete(x + xOff, y + yOff, color);
        }
}

int Renderer::loadData(const string &filename){
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        cerr << "Error opening file " + filename << endl;
        return -1;
    }
    file.seekg(0, ios::end);
    fileDataLen = file.tellg();
    file.seekg(0, ios::beg);

    fileData = new uint8_t[fileDataLen];
    file.read(reinterpret_cast<char*>(fileData), fileDataLen);
    file.close();

    string extension = filename.substr(filename.size()-3);
    if(extension == "ti2"){
        qualityFactor = 2; // 2 -> x half width
    } else if(extension == "ti4"){
        qualityFactor = 4; // 4 -> xy half res
    } else{
        qualityFactor = 1; // 1 -> regular quality
    }
    if(type == FileType::BRS){
        string offsetFilename = filename.substr(0, filename.size()-4) + ".ofs";
        std::ifstream offsetFile(offsetFilename, std::ios::binary);
        if (!offsetFile.is_open()) {
            cerr << "Error opening file " + filename << endl;
            return -1;
        }
        offsetFile.seekg(0, ios::end);
        offsetCount = offsetFile.tellg()/4;
        totalFrames = offsetCount;
        offsetFile.seekg(0, ios::beg);

        offsetData = new uint32_t[offsetCount];
        offsetFile.read(reinterpret_cast<char*>(offsetData), offsetCount*4);
        offsetFile.close();
    }
    return 0;
}

int Renderer::loadPalette(const string &filename, bool swap, int startOffset){
    uint8_t palleteFileData[0x400] = {0};

    FILE *inputPalette = fopen(filename.c_str(), "rb");
    if(inputPalette == nullptr){
        return -1;
    }
    fread(palleteFileData, 1, 0x326, inputPalette);
    fclose(inputPalette);

    for(int i=0;i<256;i++) {
        int offset = startOffset + i*3;
        if(swap) {
            graphics->setPalleteCollor(i, palleteFileData[offset] | (palleteFileData[offset + 1] << 8) | (palleteFileData[offset + 2] << 16) | 0xFF000000);
        } else {
            graphics->setPalleteCollor(i, palleteFileData[offset+2] | (palleteFileData[offset+1]<<8) | (palleteFileData[offset]<<16) | 0xFF000000);
        }
    }
    return 0;
}

int Renderer::loadPalPalette(const string &filename, bool swap){
    return loadPalette(filename, swap, 0x0A);
}

int Renderer::loadPalPaletteC(const string &filename, bool swap){
    return loadPalette(filename, swap, 0x0C);
}

int Renderer::loadDummyPalette(){
    for(int i=0;i<256;i++) {
        graphics->setPalleteCollor(i, i | 0xFF000000);
    }
}

Renderer::Renderer(Graphics *graphics, FileType type) {
    this->graphics = graphics;
    this->type = type;
}

int Renderer::getTotalFrames() {
    return totalFrames;
}

void Renderer::render(int currentFrame) {
    if(type == FileType::CBR){
        uint32_t end = currentFrame == totalFrames - 1 ? fileDataLen-4*totalFrames : offsetData[currentFrame+1];
        cbrParsePart(offsetData[currentFrame], end - offsetData[currentFrame]);
    } else if (type == FileType::PBR){
        uint32_t end = currentFrame == totalFrames - 1 ? fileDataLen-4*totalFrames : offsetData[currentFrame+1];
        pbrParsePart(offsetData[currentFrame], end - offsetData[currentFrame]);
    } else if (type == FileType::TIL){
        int realTileWidth = tileWidth;
        int realTileHeight = tileHeight;
        if(qualityFactor == 2){
            realTileWidth/=2;
        }
        if(qualityFactor == 4){
            realTileWidth/=2;
            realTileHeight/=2;
        }
        int offset = 0x320+currentFrame*delta;
        for(int l=0;l<totalWidthTiles*totalHeightTiles;l++){
            uint16_t tileId = fileData[offset] + fileData[offset + 1] * 256;
            offset+=2;

            int xOff = (l%totalWidthTiles)*realTileWidth;
            int yOff = (l/totalWidthTiles)*realTileHeight;

            tilDrawTile(tileId, realTileWidth, realTileHeight, xOff, yOff);
        }
    } else if (type == FileType::BRS) {
        int startingOffset = offsetData[currentFrame];

        uint32_t *dwords = (uint32_t*) (fileData+startingOffset);
        uint32_t w = dwords[0];
        uint32_t h = dwords[1];
        uint32_t p = dwords[2];
        uint32_t q = dwords[3];

        for(int y=0;y<h;y++) {
            for(int x=0;x<w;x++) {
                int i = x + y*w + startingOffset + 16;
                if(fileData[i] != 0)
                    graphics->setPixelPallete(x+p,y+q,fileData[i]);
            }
        }
    } else if(type == FileType::CGF) {
        int frameMetaOffset = sizeof(CGFHeader) + currentFrame * sizeof(CGFFrameMeta);

        CGFFrameMeta *frameMeta = (CGFFrameMeta*) (fileData+frameMetaOffset);

        uint32_t startOffset = sizeof(CGFHeader) + totalFrames * sizeof(CGFFrameMeta)  + frameMeta->payloadOffset;
        for(int y=0;y<frameMeta->height;y++) {
            uint32_t len = *(uint32_t*)(fileData+startOffset);
            cgfParseLine(startOffset+4, len-4, frameMeta->xOffset, frameMeta->yOffset+y);
            startOffset += len;
        }
    } else if(type == FileType::LZP){
        uint32_t offsetToOffsets = fileDataLen - totalFrames * 4 + currentFrame * 4;
        uint32_t offset = *(uint32_t*)(fileData+offsetToOffsets);
        uint32_t compressedLen = *(uint32_t*)(fileData+offset);

        uint8_t *data = (uint8_t*)(fileData+offset+4);
        uint8_t *dataEnd = (uint8_t*)(fileData+offset+4+compressedLen);

        uint8_t window[4096];
        memset(window, 0, 4096);
        int dstPos = 0;
        uint16_t N = 4095;
        uint32_t windowIndex = 4078;
        uint8_t idx = 0;
        bool end = false;
        uint8_t flags = 0;
        while(!end){
            if(idx == 0) {
                if(data >= dataEnd) break;
                flags = *data++;
            }
            if(flags & 1) {
                if(data >= dataEnd) break;
                uint8_t c = *data++;
                if(dstPos > lzpWidth * lzpHeight) break;
                graphics->setPixelPallete(dstPos%lzpWidth, dstPos/lzpWidth, c);
                dstPos++;
                window[windowIndex++] = c;
                windowIndex &= N;
            } else {
                if(data >= dataEnd) break;
                int offset = *data++;
                if(data >= dataEnd) break;
                int len = *data++;
                offset |= (len & 0xF0) << 4;
                len  = (len & 0xF) + 3;
                for(int k = 0; k < len; k++) {
                    uint8_t c = window[(offset + k) & N];
                    if(dstPos > lzpWidth * lzpHeight) {end = true; break;}
                    graphics->setPixelPallete(dstPos%lzpWidth, dstPos/lzpWidth, c);
                    dstPos++;
                    window[windowIndex++] = c;
                    windowIndex &= N;
                }
            }
            flags >>= 1;
            idx++;
            idx &= 7;
        }
    }
}

void Renderer::init() {
    if(type == FileType::CBR){
        uint32_t offsetToOffsets = *(uint32_t*)fileData;
        totalFrames = (fileDataLen-offsetToOffsets)/4;
        offsetData = (uint32_t*)(fileData+offsetToOffsets);
    } else if (type == FileType::PBR){
        uint32_t offsetToOffsets = *(uint32_t*)fileData;
        totalFrames = *(uint32_t*)(fileData + 0xC);
        offsetData = (uint32_t*)(fileData + offsetToOffsets);
    } else if (type == FileType::TIL){
        totalWidth = (fileData[8] | (fileData[9] <<8));
        totalHeight = (fileData[10] | (fileData[11] <<8));

        totalWidthTiles = fileData[0x15];
        totalHeightTiles = fileData[0x17];

        tileWidth = totalWidth/totalWidthTiles;
        tileHeight = totalHeight/totalHeightTiles;

        totalFrames = (fileData[6] | (fileData[7] <<8))+2;

        delta = totalWidthTiles*totalHeightTiles*2;
        tileDataOffset = totalFrames*delta+0x320;
    } else if(type == FileType::CGF) {
        CGFHeader *header = (CGFHeader*)(fileData);
        totalFrames = header->totalFrames;
    } else if(type == FileType::LZP){
        lzpWidth = *(uint32_t*)(fileData+4);
        lzpHeight = *(uint32_t*)(fileData+8);
        totalFrames = *(uint32_t*)(fileData+0);
    }
}

void Renderer::cgfParseLine(uint32_t idx, uint32_t len, uint32_t x, uint32_t y) {
    //printf("\n");
    printf("PARSING LINE at %x (len = %x)\n", idx, len);
    fflush(stdout);
    for(int i=0;i<len;i++){
        //printf("%x ", fileData[idx+i]);
        //continue;
        uint8_t v = fileData[idx+i];
        i++;

        if(v == 0){
            uint8_t off = fileData[idx+i];
            if(off == 0){
                return;
            }
            x += off;
        } else if(v == 1) { // UNUSED
            uint8_t count = fileData[idx+i];

            for(int j=0;j<count;j++) {
                i++;
                graphics->setPixelPallete(x++, y, fileData[idx + i]);
                i++;
                uint8_t alpha = fileData[idx + i];
            }
        } else if(v == 2) { // UNUSED
            uint8_t count = fileData[idx+i];

            i++;
            uint8_t value = fileData[idx+i];

            i++;
            uint8_t valueAlpha = fileData[idx+i];

            for(int j=0;j<count;j++) {
                graphics->setPixelPallete(x++, y, value);
            }
        } else if(v == 3){
            uint8_t count = fileData[idx+i];

            for(int j=0;j<count;j++) {
                i++;
                graphics->setPixelPallete(x++, y, fileData[idx + i]);
            }
        } else if(v == 4){
            uint8_t count = fileData[idx+i];

            i++;
            uint8_t value = fileData[idx+i];

            for(int j=0;j<count;j++) {
                graphics->setPixelPallete(x++, y, value);
            }
        } else {
            printf("Unk%x in %x\n", v, idx+i);
        }
    }
    //printf("\n");

}

void Renderer::save(int currentFrame) {
    if(type != FileType::CGF) {
        return;
    }
    int frameMetaOffset = sizeof(CGFHeader) + currentFrame * sizeof(CGFFrameMeta);

    CGFFrameMeta *frameMeta = (CGFFrameMeta*) (fileData+frameMetaOffset);

    uint32_t *data = new uint32_t[frameMeta->width * frameMeta->height];
    memset(data, 0, frameMeta->width * frameMeta->height * sizeof(uint32_t));

    uint32_t startOffset = sizeof(CGFHeader) + totalFrames * sizeof(CGFFrameMeta)  + frameMeta->payloadOffset;
    for(int y=0;y<frameMeta->height;y++) {
        uint32_t len = *(uint32_t*)(fileData+startOffset);
        cgfParseLineToBuffer(data+y*frameMeta->width, startOffset+4, len-4);
        startOffset += len;
    }

    char filename[64];
    snprintf(filename, 64, "%d_%d_%d_%d.png", currentFrame, frameMeta->xOffset, frameMeta->yOffset, frameMeta->unk3);
    stbi_write_png(filename, frameMeta->width, frameMeta->height, 4, data, frameMeta->width*4);
    delete []data;
}

void Renderer::cgfParseLineToBuffer(uint32_t *buffer, uint32_t idx, uint32_t len) {
    for(int i=0;i<len;i++){
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
}
