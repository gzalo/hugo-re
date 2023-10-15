#include "Renderer.h"
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

void Renderer::tilDrawTile(int tileId, int xOff, int yOff) {
    int offset = tileDataOffset + tileId*tileHeight*tileWidth;
    for (int y = 0; y < tileHeight; y++)
        for (int x = 0; x < tileWidth; x++) {
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

    string extension = filename.substr(filename.size()-4);
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

int Renderer::loadPalette(const string &filename, bool swap){
    uint8_t palleteFileData[0x326] = {0};

    FILE *inputPalette = fopen(filename.c_str(), "rb");
    if(inputPalette == nullptr){
        return -1;
    }
    fread(palleteFileData, 1, 0x326, inputPalette);
    fclose(inputPalette);

    for(int i=0;i<256;i++) {
        int offset = 0x20 + i*3;
        if(swap) {
            graphics->setPalleteCollor(i, palleteFileData[offset] | (palleteFileData[offset + 1] << 8) | (palleteFileData[offset + 2] << 16) | 0xFF000000);
        } else {
            graphics->setPalleteCollor(i, palleteFileData[offset+2] | (palleteFileData[offset+1]<<8) | (palleteFileData[offset]<<16) | 0xFF000000);
        }
    }
    return 0;
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
        if(qualityFactor == 2){
            tileWidth/=2;
        }
        if(qualityFactor == 4){
            tileWidth/=2;
            tileHeight/=2;
        }
        int offset = 0x320+currentFrame*delta;
        for(int l=0;l<totalWidthTiles*totalHeightTiles;l++){
            uint16_t tileId = fileData[offset] + fileData[offset + 1] * 256;
            offset+=2;

            int xOff = (l%totalWidthTiles)*tileWidth;
            int yOff = (l/totalWidthTiles)*tileHeight;

            tilDrawTile(tileId, xOff, yOff);
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

        tileDataOffset = totalFrames*totalWidthTiles*totalHeightTiles*2+0x320;
        delta = totalWidthTiles*totalHeightTiles*2;
    }
}
