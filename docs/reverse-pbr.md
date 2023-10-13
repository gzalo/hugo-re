# PBR file format documentation

Keep in mind this has been obtained by trial and error so it may contain errors:

The file contains the following blocks:
- Header
- Data0
- Data1
- ...
- Data(N-1)
- Offset table

## Header
16 bytes, containing:
- 4 bytes: offset to offset table
- 4 bytes: always 0?
- 4 bytes: always 0?
- 4 bytes: amount of frames (N)

## Data
Each data block maps to a single image, and it starts with an 8 byte header:
- 2 bytes: screen X coords of the image?
- 2 bytes: screen Y coords of the image?
- 2 bytes: width of the image (W)
- 2 bytes: height of the image (H)

Then it contains H variable length records (one per row) that follow the following format:
- 2 bytes: X offset
- 2 bytes: length of data (Z)
- Z bytes: RLE compressed data for the row (see below)

## Row data
For the data of each row, the format follows a modified RLE format:
- read byte as signed char (B)

- if positive, repeat next byte B times
- if negative, check highest 3 bits
    - if "101", it encodes a delta encoded strand that spans the next (B & 0xF) bytes (this is not always true, if highest bits are "1010" we need to include 16 to the len):
        - first byte after that one is copied straight to the output
        - next bytes are processed nibble-wise (most significant one first), and each one is a signed 4-bit delta compared to the previous byte
    - otherwise, just copy the next -B bytes straight to the output
