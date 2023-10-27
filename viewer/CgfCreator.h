#ifndef CGFCREATOR_H
#define CGFCREATOR_H

#include "Graphics.h"
#include "Renderer.h"

#define MAX_FRAMES 32

class CgfCreator {
private:
    Graphics *graphics;
    int totalFrames;

    CGFFrameMeta frameMeta[MAX_FRAMES];
    uint8_t *compressedData[MAX_FRAMES];
    size_t compressedLen[MAX_FRAMES];

public:
    CgfCreator(Graphics *graphics);
    void loadFrame(const char *filename, int i, int i1, int i2, int i3);

    int save(const char *string);

    int processRow(int w, int y, uint8_t *bff, uint8_t *pixels);
};


#endif
