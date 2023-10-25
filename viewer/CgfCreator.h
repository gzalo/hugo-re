#ifndef CGFCREATOR_H
#define CGFCREATOR_H

#include "Graphics.h"

class CgfCreator {
private:
    Graphics *graphics;
    int totalFrames;
public:
    CgfCreator(Graphics *graphics);
    void loadFrame(const char *filename, int i, int i1, int i2, int i3);

    void save(const char *string);
};


#endif CGFCREATOR_H
