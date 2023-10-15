# TIL file format documentation

Keep in mind this has been obtained by trial and error so it may contain errors:

The file contains the following blocks:
- Header (0x20 bytes)
- Pallete (0x300 bytes)
- Tile sets
- Tile data

## Header
32 bytes:
- 4 bytes unk
- 2 bytes unk
- 2 bytes tilecount-2
- 2 bytes total width (TW)
- 2 bytes total height (TH)
- 4 bytes unk

- 4 bytes unk
- 1 byte unk
- 1 byte width in tiles (W)
- 1 byte unk
- 1 byte height in tiles (H)
- 2 bytes unk
- 4 bytes unk

## Tile sets
List of W*H 2 byte numbers, each one points to a tile in the tile data. This is repeated `Tilecount` times.

## Tile data
Uncompressed tile data, in `TW/W * TH/H` byte blocks

## Low quality modes
There are some special low-quality modes in which the tile data is downsampled/mipmapped.
- Til: regular quality
- Ti2: X half width
- Ti4: X and Y half width