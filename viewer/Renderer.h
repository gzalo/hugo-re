
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
    BRS,
    CGF,
    LZP
};

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

    // LZP
    int lzpWidth;
    int lzpHeight;

    int8_t pbrGetHighContrib(uint8_t x);
    int8_t pbrGetLowContrib(uint8_t x);
    void pbrUnpackbits(uint32_t idx, int largo, int startingOffset, int y);
    void pbrParsePart(uint32_t start, uint32_t len);

    void cbrUnpackbits(uint32_t idx, int largo, int startingOffset, int x, int y);
    void cbrParsePart(uint32_t start, uint32_t len);

    void tilDrawTile(int tileId, int tileRealWidth, int tileRealHeight, int xOff, int yOff);
    void cgfParseLine(uint32_t idx, uint32_t len, uint32_t p, uint32_t q);

    public:
    Renderer(Graphics *graphics, FileType type);
    int loadPalette(const string &filename, bool swap, int startOffset = 0x20);
    int loadPalPalette(const string &filename, bool swap);
    int loadPalPaletteC(const string &filename, bool swap);
    int loadDummyPalette();
    int loadData(const string &filename);
    int getTotalFrames();
    int loadBmpPallete(const string &filename);
    int loadCgfPallete(const string &str);

    void render(int i);

    void init();

    void save(int i);

    void cgfParseLineToBuffer(uint32_t *string, uint32_t i, uint32_t i1);

};

#endif