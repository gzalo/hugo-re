package com.gzalo.hugore.converter;

import java.lang.reflect.InvocationTargetException;

public enum FileType {
    CGF(CgfConverter.class),
    BMP(NoOpConverter.class),
    TIL(TilConverter.class),
    WAV(NoOpConverter.class),
    OOS(OosConverter.class),
    TXT(NoOpConverter.class),
    IFF(NoOpConverter.class), // Amiga format, redundant
    OLD(NoOpConverter.class),
    LZP(LzpConverter.class),
    RAW(NoOpConverter.class), // Not used too much
    LBM(NoOpConverter.class),
    BAK(NoOpConverter.class),
    PCX(NoOpConverter.class), // Not really used
    PAL(NoOpConverter.class),  // Only used for menu
    FLC(NoOpConverter.class), // Used just once
    CEP(NoOpConverter.class), // ITE CORE EXPLORER PROJECT
    SCC(NoOpConverter.class), // Visual SourceSafe
    AL(NoOpConverter.class); // Text palettes

    private final Class<? extends Converter> converter;

    FileType(Class<? extends Converter> converter) {
        this.converter = converter;
    }

    public Converter getConverter() {
        try {
            return converter.getDeclaredConstructor().newInstance();
        } catch (InstantiationException | IllegalAccessException | InvocationTargetException | NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
    }
}
