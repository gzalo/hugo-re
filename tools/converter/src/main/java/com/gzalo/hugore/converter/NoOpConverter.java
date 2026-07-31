package com.gzalo.hugore.converter;

import org.apache.commons.io.FileUtils;

import java.io.IOException;
import java.nio.file.Path;

public class NoOpConverter implements Converter {
    @Override
    public int convert(Path input, Path output) {
        try {
            FileUtils.copyFile(input.toFile(), output.toFile());
            return 0;
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
