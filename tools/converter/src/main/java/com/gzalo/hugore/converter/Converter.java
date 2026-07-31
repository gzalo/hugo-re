package com.gzalo.hugore.converter;

import java.io.IOException;
import java.nio.file.Path;

public interface Converter {
    int convert(Path input, Path output) throws IOException;
}
