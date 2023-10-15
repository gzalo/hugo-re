
#ifndef RENDERER_H
#define RENDERER_H

#include <cstdint>
#include <string>
#include "Graphics.h"
using namespace std;

enum class FileType {
    TIL,
    PBR,
    CBR,
    BRS
};

class Renderer {
private:
    Graphics *graphics;
    FileType type;
    int totalFrames;
    int qualityFactor;
    uint8_t *fileData;
    size_t fileDataLen;
    uint32_t *offsetData;
    size_t offsetCount;

    // TIL
    int totalWidth;
    int totalHeight;
    int totalWidthTiles;
    int totalHeightTiles;
    int tileWidth;
    int tileHeight;
    int tileDataOffset;
    int delta;

    int8_t pbrGetHighContrib(uint8_t x);
    int8_t pbrGetLowContrib(uint8_t x);
    void pbrUnpackbits(uint32_t idx, int largo, int startingOffset, int y);
    void pbrParsePart(uint32_t start, uint32_t len);

    void cbrUnpackbits(uint32_t idx, int largo, int startingOffset, int x, int y);
    void cbrParsePart(uint32_t start, uint32_t len);

    void tilDrawTile(int tileId, int xOff, int yOff);

    public:
    Renderer(Graphics *graphics, FileType type);
    int loadPalette(const string &filename, bool swap);
    int loadData(const string &filename);
    int getTotalFrames();

    void render(int i);

    void init();
};

#endif