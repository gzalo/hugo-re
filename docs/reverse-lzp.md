# LZP format

## 32 byte header 
u32 compressed block count (N)
u32 width
u32 height
u32 frame count?
u32 unk
...

## 256*3 byte palette

## Compressed data (N blocks back to back) 
- u32 L len of data
- L bytes
- LZSS compressed, 1 byte per pixel

LZSS params:
- blocks start with a byte that tells if any of the following 8 parts are literal (if bit is set) or look-back (if bit is clear), LSB first
- if literal, data is copied as-is
- if look-back, it takes 2 bytes 
    - first byte: location (low part)
    - high nibble of second byte: location (high part)
    - low nibble of second byte: size minus two
- 4096 bytes of window
- starting index when writing: 4076

## Footer
N u32 pointing to the start of each compressed block