package com.gzalo.hugore.converter;

import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

public class OosConverter implements Converter {

    public record OosData(
        List<Integer> frameIndices
    ){
    };

    @Override
    public int convert(Path input, Path output) {
        try {
            byte[] fileContent = Files.readAllBytes(input);

            int LEN_START_PTR = 0x14;
            int DATA_START_PTR = 0x18;
            int offsetToLength = BinUtils.getInt(fileContent, LEN_START_PTR);
            int offsetToData = BinUtils.getInt(fileContent, DATA_START_PTR);
            int length = BinUtils.getInt(fileContent, offsetToLength);
            List<Integer> list = new ArrayList<>();
            for(int i=0; i<length; i++) {
                int frameId = fileContent[offsetToData + i];
                list.add(frameId);
            }
            final OosData data = new OosData(list);
            final Path outputWithJsonExtension = output.resolveSibling(output.getFileName() + ".json");
            outputWithJsonExtension.toFile().getParentFile().mkdirs();
            new ObjectMapper().writeValue(outputWithJsonExtension.toFile(), data);
            return 0;
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
