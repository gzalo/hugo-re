package com.gzalo.hugore.converter;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

public class LzpConverter implements Converter {
    @Override
    public int convert(Path input, Path output) throws IOException {
        byte[] fileContent = Files.readAllBytes(input);
        if(fileContent.length == 0){
            return 0;
        }
        Palette palette = Palette.fromData(fileContent, 0x20);

        int totalFrames = BinUtils.getInt(fileContent, 0);
        int width = BinUtils.getInt(fileContent, 4);
        int height = BinUtils.getInt(fileContent, 8);

        for(int currentFrame = 0;currentFrame < totalFrames;currentFrame++) {
            int offsetToOffsets = fileContent.length - totalFrames * 4 + currentFrame * 4;
            int offsetLocal = BinUtils.getInt(fileContent, offsetToOffsets);
            int compressedLen = BinUtils.getInt(fileContent, offsetLocal);
            int[] data = new int[width * height];

            int dataPtr = offsetLocal + 4;
            final int dataEnd = dataPtr + compressedLen;

            int[] window = new int[4096];
            int dstPos = 0;
            final int N = 4095;
            int windowIndex = 4078;
            int idx = 0;
            boolean end = false;
            int flags = 0;
            while (!end) {
                if (idx == 0) {
                    if (dataPtr >= dataEnd) break;
                    flags = fileContent[dataPtr++] & 0xFF;
                }
                if ((flags & 1) != 0) {
                    if (dataPtr >= dataEnd) break;
                    int c = fileContent[dataPtr++] & 0xFF;
                    if (dstPos > width * height) break;
                    data[dstPos % width + (dstPos/width) * width] = palette.colors()[c];
                    dstPos++;
                    window[windowIndex++] = c;
                    windowIndex &= N;
                } else {
                    if (dataPtr >= dataEnd) break;
                    int offset = fileContent[dataPtr++] & 0xFF;
                    if (dataPtr >= dataEnd) break;
                    int len = fileContent[dataPtr++] & 0xFF;
                    offset |= (len & 0xF0) << 4;
                    len = (len & 0xF) + 3;
                    for (int k = 0; k < len; k++) {
                        int c = window[(offset + k) & N];
                        if (dstPos > width * height) {
                            end = true;
                            break;
                        }
                        data[dstPos % width + (dstPos/width) * width] = palette.colors()[c];
                        dstPos++;
                        window[windowIndex++] = c;
                        windowIndex &= N;
                    }
                }
                flags >>= 1;
                idx++;
                idx &= 7;
            }

            final Path outputWithPngExtension = output.resolveSibling(output.getFileName() + "_" + currentFrame + ".png");
            outputWithPngExtension.toFile().getParentFile().mkdirs();
            PngUtils.convertARGBToPng(data, width, height, outputWithPngExtension.toFile());
        }
        return 0;
    }
}
