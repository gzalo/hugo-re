#include "CgfCreator.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int CgfCreator::processRow(int w, int y, uint8_t *bff, uint8_t *pixels) {
    uint16_t firstCompression[w];
    size_t firstLen = 0;

#if 0
    // No alpha uncompressed
    if(w >= 256) exit(-1);
    int bbbb = 0;
    bff[bbbb++] = 3;
    bff[bbbb++] = w;
    for (int i = 0; i < w; i++){
        uint8_t p = pixels[i+y*w];
        bff[bbbb++] = p == 0xFF ? 0xFF : p;
    }
    bff[bbbb++] = 0;
    bff[bbbb++] = 0;
    return bbbb;
#endif


#if 0
    printf("%d\n", y);
    for(int i=0;i<w;i++) {
        printf("%x ", pixels[i+y*w]);
    }
    printf("\n");
#endif

    uint8_t prev = pixels[0+y*w];
    int count = 1;
    for(int x=1;x<w;x++){
        uint8_t current = pixels[x+y*w];
        if( current == prev){
            count++;
        } else {
            uint16_t val = (count << 8) | prev;
            firstCompression[firstLen++] = val;
            prev = current;
            count = 1;
        }
    }
    firstCompression[firstLen++] =  (count << 8) | prev;

/*
    for(int i=0;i<firstLen;i++) {
        printf("%x ", firstCompression[i]);
    }
    printf("\n");*/

    int bytes = 0;
    for(int i=0;i<firstLen;i++){
        uint16_t current = firstCompression[i];
        uint8_t runLength = current >> 8;
        uint8_t runVal = current & 0xFF;
        if(/*runLength > 2 || */runVal == 0xFF){
            if(runVal == 0xFF) {
                bff[bytes++] = 0;
                bff[bytes++] = i == firstLen-1 ? 0 : runLength;
            } else {
                bff[bytes++] = 4;
                bff[bytes++] = runLength;
                bff[bytes++] = runVal;
            }
        } else {
            int j=i+1;
            int eq = firstCompression[i]>>8;
            while(j<firstLen && (firstCompression[j]&0xFF) != 0xFF/* && (firstCompression[j]>>8) <= 2*/){
                eq += firstCompression[j]>>8;
                j++;
            }
            bff[bytes++] = 3;
            bff[bytes++] = eq;
            for(int k=i;k<j;k++) {
                uint8_t nrl = firstCompression[k] >> 8;
                uint8_t nrv = firstCompression[k] & 0xFF;
                for(int r=0;r<nrl;r++)
                    bff[bytes++] = nrv;
            }
            i = j-1;
        }
    }

    bff[bytes++] = 0;
    bff[bytes++] = 0;

    // Transp => 0 LEN
    // Dist => 3 LEN x y z
    // EQ => 4 LEN VAL
    return bytes;
}

void CgfCreator::loadFrame(const char *filename, int frameIndex, int xOffset, int yOffset, int unk3) {
    int w, h, ch;
    uint32_t *data = (uint32_t*) stbi_load(filename, &w, &h, &ch, 4);

    uint8_t *pixels = new uint8_t[w*h];
    for(int i=0;i<w*h;i++){
        pixels[i] = graphics->getColor(data[i]);
    }

    frameMeta[totalFrames].xOffset = xOffset;
    frameMeta[totalFrames].yOffset = yOffset;
    frameMeta[totalFrames].width = w;
    frameMeta[totalFrames].height = h;
    frameMeta[totalFrames].unk3 = unk3;

    compressedData[totalFrames] = new uint8_t[w*h*3]; // Just in case
    uint8_t *currData = compressedData[totalFrames];

    int index = 0;
    for(int y=0;y<h;y++) {
        uint32_t *rowLen = (uint32_t*)(currData+index);
        int usedBytes = processRow(w, y, currData+index+4, pixels);

        printf("Produced: ");
        for(int i=0;i<usedBytes;i++){
            printf("%x ", currData[index+i+4]);
        }
        printf("\n");

        index += usedBytes + 4;
        *rowLen = usedBytes + 4;
    }

    printf("\n");

    compressedLen[totalFrames] = index;

    free(data);
    totalFrames++;
}
CgfCreator::CgfCreator(Graphics *graphics) {
    this->graphics = graphics;
    this->totalFrames = 0;
}

int CgfCreator::save(const char *filename, uint32_t unk1, uint32_t unk2) {
    CGFHeader header = (CGFHeader) {
            .magic = 0x46464743,
            .flags = 1,
            .totalFrames = (uint32_t) totalFrames,
            .frameMetadataSize = (uint32_t) sizeof(CGFFrameMeta) * totalFrames,
            .framePayloadSize = 0,
            .unk1 = unk1,
            .unk2 = unk2
    };

    for(int i=0;i<totalFrames;i++) {
        header.framePayloadSize += compressedLen[i];
    }

    FILE *fp = fopen(filename, "wb");
    if(!fp){
        return -1;
    }

    fwrite(&header, sizeof(CGFHeader), 1, fp);
    size_t offset = 0;
    for(int i=0;i<totalFrames;i++){
        frameMeta[i].payloadOffset = offset;
        offset += compressedLen[i];
    }
    fwrite(frameMeta, sizeof(CGFFrameMeta), totalFrames, fp);

    for (int i = 0; i < totalFrames; i++) {
        printf("Frame %d Len %d\n", i, compressedLen[i]);
        fwrite(compressedData[i], compressedLen[i], 1, fp);
    }

    fclose(fp);
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
