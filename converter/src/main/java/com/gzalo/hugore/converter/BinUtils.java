package com.gzalo.hugore.converter;

public class BinUtils {
    public static int getInt(byte[] bytes, int index) {
        return (bytes[index] & 0xFF) |
                ((bytes[index + 1] & 0xFF) << 8) |
                ((bytes[index + 2] & 0xFF) << 16) |
                ((bytes[index + 3] & 0xFF) << 24);
    }

    public static short getShort(byte[] bytes, int index) {
        return (short) ((bytes[index] & 0xFF) |
                        ((bytes[index + 1] & 0xFF) << 8));
    }

    public static int getRGB(byte[] bytes, int index) {
        return (bytes[index] & 0xFF) |
                ((bytes[index + 1] & 0xFF) << 8) |
                ((bytes[index + 2] & 0xFF) << 16);
    }

    public static int getBGR(byte[] bytes, int index) {
        return (bytes[index + 2] & 0xFF) |
                ((bytes[index + 1] & 0xFF) << 8) |
                ((bytes[index] & 0xFF) << 16);
    }
}
