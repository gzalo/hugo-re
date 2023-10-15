# CBR file format documentation

Keep in mind this has been obtained by trial and error so it may contain errors:

The file contains the following blocks:
- Header
- Data0
- Data1
- ...
- Data(N-1)
- Offset table

## Header
12 bytes, containing:
- 4 bytes: offset to offset table
- 4 bytes: unknown
- 4 bytes: unknown

## Data
Each data block maps to a single image, and it starts with an 16 byte header:
- 4 bytes: width of the image (W)
- 4 bytes: height of the image (H)
- 4 bytes: screen X coords of the image
- 4 bytes: screen Y coords of the image

Then it contains H variable length records (one per row) that follow the following format:
- 2 bytes: X offset
- 2 bytes: length of data (Z)
- Z bytes: Data for the row, uncompressed, each byte maps to an element of the pallete
