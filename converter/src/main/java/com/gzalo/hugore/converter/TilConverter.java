package com.gzalo.hugore.converter;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

public class TilConverter implements Converter {
    @Override
    public int convert(Path input, Path output) throws IOException {
        byte[] fileContent = Files.readAllBytes(input);
        Palette palette = Palette.fromData(fileContent, 0x20);

        int totalWidth = BinUtils.getShort(fileContent, 8);
        int totalHeight = BinUtils.getShort(fileContent, 10);
        int totalWidthTiles = fileContent[0x15];
        int totalHeightTiles = fileContent[0x17];
        int tileWidth = totalWidth / totalWidthTiles;
        int tileHeight = totalHeight / totalHeightTiles;
        int totalFrames = BinUtils.getShort(fileContent, 6) + 2;
        int delta = totalWidthTiles * totalHeightTiles * 2;
        int tileDataOffset = totalFrames * delta + 0x320;

        for (int currentFrame = 0; currentFrame < totalFrames; currentFrame++) {
            int offset = 0x320 + currentFrame * delta;
            int[] data = new int[totalWidth * totalHeight];

            for (int l = 0; l < totalWidthTiles * totalHeightTiles; l++) {
                short tileId = BinUtils.getShort(fileContent, offset);
                offset += 2;

                int xOff = (l % totalWidthTiles) * tileWidth;
                int yOff = (l / totalWidthTiles) * tileHeight;

                int currOffset = tileDataOffset + tileId * tileHeight * tileWidth;
                for (int y = 0; y < tileHeight; y++)
                    for (int x = 0; x < tileWidth; x++) {
                        if (currOffset >= fileContent.length) {
                            return 1;
                        }
                        int color = fileContent[currOffset++] & 0xFF;
                        data[x + xOff + (y + yOff) * totalWidth] = palette.colors()[color];
                    }
            }
            final Path outputWithPngExtension = output.resolveSibling(output.getFileName() + "_" + currentFrame + ".png");
            outputWithPngExtension.toFile().getParentFile().mkdirs();
            PngUtils.convertARGBToPng(data, totalWidth, totalHeight, outputWithPngExtension.toFile());
        }

        return 0;
    }
}
