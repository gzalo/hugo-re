package com.gzalo.hugore.converter;

public record Palette(int[] colors){
    public static Palette fromData(byte[] data, int offset){
        int[] colors = new int[256];
        for(int i=0;i<256;i++){
            colors[i] = BinUtils.getBGR(data, offset) | 0xFF000000;
            offset+=3;
        }
        return new Palette(colors);
    }

    public static Palette fromAlphaData(byte[] data, int offset) {
        int[] colors = new int[256];
        for(int i=0;i<256;i++){
            colors[i] = BinUtils.getInt(data, offset) | 0xFF000000;
            offset+=4;
        }
        return new Palette(colors);
    }
}
