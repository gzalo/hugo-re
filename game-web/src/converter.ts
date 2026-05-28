/**
 * Port of game/converter.py.
 * Decodes CGF, TIL, LZP sprite files and OOS sync files into browser-native types.
 */

// ─── helpers ─────────────────────────────────────────────────────────────────

function getInt(data: Uint8Array, offset: number): number {
  return (
    data[offset] |
    (data[offset + 1] << 8) |
    (data[offset + 2] << 16) |
    ((data[offset + 3] << 24) >> 0)  // sign-extend to match Python struct '<i'
  );
}

function getShort(data: Uint8Array, offset: number): number {
  const v = data[offset] | (data[offset + 1] << 8);
  return v >= 0x8000 ? v - 0x10000 : v;  // sign-extend to match '<h'
}

type RGBA = [number, number, number, number];

function paletteFromData(data: Uint8Array, offset: number): RGBA[] {
  const colors: RGBA[] = [];
  for (let i = 0; i < 256; i++) {
    colors.push([data[offset], data[offset + 1], data[offset + 2], 255]);
    offset += 3;
  }
  return colors;
}

function paletteFromAlphaData(data: Uint8Array, offset: number): RGBA[] {
  const colors: RGBA[] = [];
  for (let i = 0; i < 256; i++) {
    // Java: getInt(data, offset) | 0xFF000000 stored as ARGB packed int
    // ARGB: A=FF, R=data[offset+2], G=data[offset+1], B=data[offset]
    colors.push([data[offset + 2], data[offset + 1], data[offset], 255]);
    offset += 4;
  }
  return colors;
}

function makeBitmap(pixels: Uint8ClampedArray, width: number, height: number): ImageBitmap {
  const imageData = new ImageData(pixels as Uint8ClampedArray<ArrayBuffer>, width, height);
  // createImageBitmap is synchronous-ish via Promise but we pre-await it
  // Caller must await this
  return createImageBitmap(imageData) as unknown as ImageBitmap;
}

async function makeBitmapAsync(pixels: Uint8ClampedArray, width: number, height: number): Promise<ImageBitmap> {
  const imageData = new ImageData(pixels as Uint8ClampedArray<ArrayBuffer>, width, height);
  return createImageBitmap(imageData);
}

// ─── CGF ─────────────────────────────────────────────────────────────────────

function cgfParseLine(
  pixels: Uint8ClampedArray,
  data: Uint8Array,
  idx: number,
  length: number,
  x: number,
  y: number,
  palette: RGBA[],
  width: number,
): void {
  let i = 0;
  while (i < length) {
    const v = data[idx + i];
    i += 1;  // skip past opcode

    if (v === 0) {
      const off = data[idx + i];
      if (off === 0) return;
      x += off;
    } else if (v === 1) {
      const count = data[idx + i];
      for (let c = 0; c < count; c++) {
        i += 1;
        const palIdx = data[idx + i];
        const pos = x + width * y;
        pixels.set(palette[palIdx], pos * 4);
        x += 1;
        i += 1;  // extra i++ present in case v=1
      }
    } else if (v === 2) {
      const count = data[idx + i];
      i += 1;
      const value = data[idx + i];
      i += 1;
      // value_alpha would be data[idx + i]; read to advance i, not used
      for (let c = 0; c < count; c++) {
        const pos = x + width * y;
        pixels.set(palette[value], pos * 4);
        x += 1;
      }
    } else if (v === 3) {
      const count = data[idx + i];
      for (let c = 0; c < count; c++) {
        i += 1;
        const palIdx = data[idx + i];
        const pos = x + width * y;
        pixels.set(palette[palIdx], pos * 4);
        x += 1;
      }
    } else if (v === 4) {
      const count = data[idx + i];
      i += 1;
      const value = data[idx + i];
      for (let c = 0; c < count; c++) {
        const pos = x + width * y;
        pixels.set(palette[value], pos * 4);
        x += 1;
      }
    } else {
      console.warn(`Unknown byte in CGF: ${v} @ ${idx + i}`);
    }

    i += 1;  // outer for-loop increment
  }
}

async function decodeCgf(fileContent: Uint8Array, tilData: Uint8Array | null): Promise<ImageBitmap[]> {
  let palette: RGBA[];

  if (tilData !== null) {
    palette = paletteFromData(tilData, 0x20);
  } else if (fileContent.length >= 0x400) {
    palette = paletteFromAlphaData(fileContent, fileContent.length - 0x400);
  } else {
    console.warn('Cannot find feasible palette for CGF file');
    return [];
  }

  const totalFrames = getInt(fileContent, 8);
  const bitmaps: ImageBitmap[] = [];

  for (let frameIdx = 0; frameIdx < totalFrames; frameIdx++) {
    const metaOffset = 7 * 4 + frameIdx * 6 * 4;
    const width  = getInt(fileContent, metaOffset + 8);
    const height = getInt(fileContent, metaOffset + 12);
    const payloadOffset = getInt(fileContent, metaOffset + 20);

    if (width < 0 || height < 0) {
      console.warn(`Invalid frame size: ${width}x${height}`);
      return [];
    }

    if (width === 0 || height === 0) {
      bitmaps.push(await createImageBitmap(new ImageData(new Uint8ClampedArray(4), 1, 1)));
      continue;
    }

    const pixels = new Uint8ClampedArray(width * height * 4);  // zero = fully transparent
    let startOffset = 7 * 4 + totalFrames * 6 * 4 + payloadOffset;

    for (let y = 0; y < height; y++) {
      const lineLen = getInt(fileContent, startOffset);
      cgfParseLine(pixels, fileContent, startOffset + 4, lineLen - 4, 0, y, palette, width);
      startOffset += lineLen;
    }

    bitmaps.push(await makeBitmapAsync(pixels, width, height));
  }

  return bitmaps;
}

// ─── TIL ─────────────────────────────────────────────────────────────────────

async function decodeTil(fileContent: Uint8Array): Promise<ImageBitmap[]> {
  const palette = paletteFromData(fileContent, 0x20);

  const totalWidth  = getShort(fileContent, 8) & 0xFFFF;
  const totalHeight = getShort(fileContent, 10) & 0xFFFF;
  const totalWidthTiles  = fileContent[0x15];
  const totalHeightTiles = fileContent[0x17];
  const tileWidth  = Math.floor(totalWidth  / totalWidthTiles);
  const tileHeight = Math.floor(totalHeight / totalHeightTiles);
  const totalFrames = (getShort(fileContent, 6) & 0xFFFF) + 2;
  const delta = totalWidthTiles * totalHeightTiles * 2;
  const tileDataOffset = totalFrames * delta + 0x320;

  const bitmaps: ImageBitmap[] = [];

  for (let frameIdx = 0; frameIdx < totalFrames; frameIdx++) {
    let frameOffset = 0x320 + frameIdx * delta;
    const pixels = new Uint8ClampedArray(totalWidth * totalHeight * 4);
    let valid = true;

    for (let l = 0; l < totalWidthTiles * totalHeightTiles; l++) {
      const tileId = getShort(fileContent, frameOffset) & 0xFFFF;
      frameOffset += 2;
      const xOff = (l % totalWidthTiles) * tileWidth;
      const yOff = Math.floor(l / totalWidthTiles) * tileHeight;
      let currOffset = tileDataOffset + tileId * tileHeight * tileWidth;

      outer: for (let y = 0; y < tileHeight; y++) {
        for (let x = 0; x < tileWidth; x++) {
          if (currOffset >= fileContent.length) { valid = false; break outer; }
          const colorIdx = fileContent[currOffset++];
          const pos = (x + xOff) + (y + yOff) * totalWidth;
          pixels.set(palette[colorIdx], pos * 4);
        }
      }
      if (!valid) break;
    }

    if (!valid) break;
    bitmaps.push(await makeBitmapAsync(pixels, totalWidth, totalHeight));
  }

  return bitmaps;
}

// ─── LZP ─────────────────────────────────────────────────────────────────────

async function decodeLzp(fileContent: Uint8Array): Promise<ImageBitmap[]> {
  if (fileContent.length === 0) return [];

  const palette = paletteFromData(fileContent, 0x20);
  const totalFrames = getInt(fileContent, 0);
  const width  = getInt(fileContent, 4);
  const height = getInt(fileContent, 8);
  const N = 4095;

  const bitmaps: ImageBitmap[] = [];

  for (let frameIdx = 0; frameIdx < totalFrames; frameIdx++) {
    const offsetToOffsets = fileContent.length - totalFrames * 4 + frameIdx * 4;
    const offsetLocal = getInt(fileContent, offsetToOffsets);
    const compressedLen = getInt(fileContent, offsetLocal);

    const pixels = new Uint8ClampedArray(width * height * 4);
    let dataPtr = offsetLocal + 4;
    const dataEnd = dataPtr + compressedLen;

    const window = new Uint8Array(4096);
    let dstPos = 0;
    let windowIndex = 4078;
    let bitIdx = 0;
    let end = false;
    let flags = 0;

    while (!end) {
      if (bitIdx === 0) {
        if (dataPtr >= dataEnd) break;
        flags = fileContent[dataPtr++];
      }

      if ((flags & 1) !== 0) {
        if (dataPtr >= dataEnd) break;
        const c = fileContent[dataPtr++];
        if (dstPos > width * height) break;
        const pos = dstPos % width + Math.floor(dstPos / width) * width;
        pixels.set(palette[c], pos * 4);
        dstPos++;
        window[windowIndex] = c;
        windowIndex = (windowIndex + 1) & N;
      } else {
        if (dataPtr >= dataEnd) break;
        const offset = fileContent[dataPtr++];
        if (dataPtr >= dataEnd) break;
        const lengthByte = fileContent[dataPtr++];
        const backOffset = offset | ((lengthByte & 0xF0) << 4);
        const runLen = (lengthByte & 0xF) + 3;
        for (let k = 0; k < runLen; k++) {
          const c = window[(backOffset + k) & N];
          if (dstPos > width * height) { end = true; break; }
          const pos = dstPos % width + Math.floor(dstPos / width) * width;
          pixels.set(palette[c], pos * 4);
          dstPos++;
          window[windowIndex] = c;
          windowIndex = (windowIndex + 1) & N;
        }
      }

      flags >>= 1;
      bitIdx = (bitIdx + 1) & 7;
    }

    bitmaps.push(await makeBitmapAsync(pixels, width, height));
  }

  return bitmaps;
}

// ─── OOS ─────────────────────────────────────────────────────────────────────

export function decodeOos(data: Uint8Array): number[] {
  const offsetToLength = getInt(data, 0x14);
  const offsetToData   = getInt(data, 0x18);
  const length = getInt(data, offsetToLength);
  const result: number[] = [];
  for (let i = 0; i < length; i++) {
    result.push(data[offsetToData + i]);
  }
  return result;
}

// ─── Public API ──────────────────────────────────────────────────────────────

const surfaceCache = new Map<string, ImageBitmap[]>();
const oosCache     = new Map<string, number[]>();

/**
 * Decode a CGF, TIL, or LZP file to ImageBitmap[].
 * Pass the raw file bytes and optionally sibling TIL bytes for palette lookup (CGF only).
 */
export async function decodeSurfaces(
  filePath: string,
  fileContent: Uint8Array,
  tilData: Uint8Array | null = null,
): Promise<ImageBitmap[]> {
  if (surfaceCache.has(filePath)) return surfaceCache.get(filePath)!;

  const ext = filePath.split('.').pop()?.toUpperCase() ?? '';
  let surfaces: ImageBitmap[];

  if (ext === 'CGF') {
    surfaces = await decodeCgf(fileContent, tilData);
  } else if (ext === 'TIL') {
    surfaces = await decodeTil(fileContent);
  } else if (ext === 'LZP') {
    surfaces = await decodeLzp(fileContent);
  } else {
    console.warn(`Unsupported format for decodeSurfaces: ${ext}`);
    surfaces = [];
  }

  surfaceCache.set(filePath, surfaces);
  return surfaces;
}

/** Decode an OOS sync file to a frame-index array. */
export function decodeSyncData(filePath: string, data: Uint8Array): number[] {
  if (oosCache.has(filePath)) return oosCache.get(filePath)!;
  const result = decodeOos(data);
  oosCache.set(filePath, result);
  return result;
}
