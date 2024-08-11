package com.gzalo.hugore.converter;

import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

public class CgfConverter implements Converter {

    private void cgfParseLine(int[] out, byte[] data, int idx, int len, int x, int y, Palette palette, int width) {
        for(int i=0;i<len;i++){
            int v = data[idx+i] & 0xFF;
            i++;

            if(v == 0){
                int off = data[idx+i] & 0xFF;
                if(off == 0){
                    return;
                }
                x += off;
            } else if(v == 1) { // UNUSED
                int count = data[idx+i] & 0xFF;

                for(int j=0;j<count;j++) {
                    i++;
                    int pallete = data[idx+i] & 0xFF;
                    out[x + width*y] = palette.colors()[pallete];
                    x++;
                    i++;
                }
            } else if(v == 2) { // UNUSED
                int count = data[idx+i] & 0xFF;

                i++;
                int value = data[idx+i] & 0xFF;

                i++;
                int valueAlpha = data[idx+i] & 0xFF;

                for(int j=0;j<count;j++) {
                    out[x + width*y] = palette.colors()[value];
                    x++;
                }
            } else if(v == 3){
                int count = data[idx+i] & 0xFF;

                for(int j=0;j<count;j++) {
                    i++;
                    int pallete = data[idx+i] & 0xFF;
                    out[x + width*y] = palette.colors()[pallete];
                    x++;
                }
            } else if(v == 4){
                int count = data[idx+i] & 0xFF;

                i++;
                int value = data[idx+i] & 0xFF;

                for(int j=0;j<count;j++) {

                    out[x + width * y] = palette.colors()[value];
                    x++;
                }
            } else {
                System.out.println("Unknown byte in CGF: " + v + " @ " + idx+i);
            }
        }

    }

    public record CgfData(
            List<Integer> xOffsets,
            List<Integer> yOffsets
    ){
    };


    @Override
    public int convert(Path input, Path output) throws IOException {
        byte[] fileContent = Files.readAllBytes(input);

        if(fileContent.length < 0x400) {
            System.out.println(input + " is too short");
            return 1;
        }
        Palette palette = Palette.fromAlphaData(fileContent, fileContent.length-0x400);

        int totalFrames = BinUtils.getInt(fileContent, 8);
        List<Integer> xOffsets = new ArrayList<>();
        List<Integer> yOffsets = new ArrayList<>();

        for(int currentFrame = 0; currentFrame < totalFrames; currentFrame++) {
            int frameMetaOffset = 7*4 + currentFrame * 6*4;
            int xOffset = BinUtils.getInt(fileContent, frameMetaOffset);
            int yOffset = BinUtils.getInt(fileContent, frameMetaOffset+4);
            int width = BinUtils.getInt(fileContent, frameMetaOffset+8);
            int height = BinUtils.getInt(fileContent, frameMetaOffset+12);
            int payloadOffset = BinUtils.getInt(fileContent, frameMetaOffset+20);

            xOffsets.add(xOffset);
            yOffsets.add(yOffset);

            if(height == 0 || width == 0) {
                System.out.println("Frame size is zero in " + input + " at " + currentFrame);
                continue;
            }
            int[] data = new int[width*height];

            int startOffset = 7*4 + totalFrames * 6*4 + payloadOffset;
            for(int y=0;y<height;y++) {
                int len = BinUtils.getInt(fileContent, startOffset);
                cgfParseLine(data, fileContent, startOffset+4, len-4, 0, y, palette, width);
                startOffset += len;
            }

            final Path outputWithPngExtension = output.resolveSibling(output.getFileName() + "_" + currentFrame + ".png");
            outputWithPngExtension.toFile().getParentFile().mkdirs();
            PngUtils.convertARGBToPng(data, width, height, outputWithPngExtension.toFile());

        }
        final CgfData data = new CgfData(xOffsets, yOffsets);
        final Path outputWithJsonExtension = output.resolveSibling(output.getFileName() + ".json");
        outputWithJsonExtension.toFile().getParentFile().mkdirs();
        new ObjectMapper().writeValue(outputWithJsonExtension.toFile(), data);
        return 0;
    }
}
