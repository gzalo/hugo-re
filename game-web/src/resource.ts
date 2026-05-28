/**
 * Resource loader — mirrors game/resource.py.
 *
 * External game data (CGF/TIL/LZP/OOS/WAV) is accessed via the File System
 * Access API after the user picks their DATA_DIR once.
 *
 * Assets in game-web/resources/ (images, shaders, videos, music, splats) are
 * fetched from the server via HTTP as usual.
 */

import { decodeSurfaces, decodeSyncData } from './converter';

// ─── File System Access API root ─────────────────────────────────────────────

let _dataDir: FileSystemDirectoryHandle | null = null;

export function setDataDir(handle: FileSystemDirectoryHandle): void {
  _dataDir = handle;
}

async function readFromDataDir(parts: string[]): Promise<Uint8Array> {
  if (!_dataDir) throw new Error('DATA_DIR not set — call setDataDir() first');
  let dir: FileSystemDirectoryHandle = _dataDir;
  for (let i = 0; i < parts.length - 1; i++) {
    dir = await dir.getDirectoryHandle(parts[i]);
  }
  const fileHandle = await dir.getFileHandle(parts[parts.length - 1]);
  const file = await fileHandle.getFile();
  return new Uint8Array(await file.arrayBuffer());
}

/** Try to find a sibling .TIL file in the same directory as a CGF file. */
async function findSiblingTil(
  game: string,
  gfxDir: string,
): Promise<Uint8Array | null> {
  if (!_dataDir) return null;
  try {
    let dir: FileSystemDirectoryHandle = _dataDir;
    for (const part of [game, gfxDir]) {
      dir = await dir.getDirectoryHandle(part);
    }
    for await (const [name] of (dir as any).entries()) {
      if (typeof name === 'string' && name.toLowerCase().endsWith('.til')) {
        const fh = await dir.getFileHandle(name);
        const file = await fh.getFile();
        return new Uint8Array(await file.arrayBuffer());
      }
    }
  } catch {
    // directory may not exist
  }
  return null;
}

// ─── Surface loading ─────────────────────────────────────────────────────────

function gfxDir(game: string): string {
  return game === 'RopeOutroData' ? 'GFX' : 'gfx';
}

/**
 * Load a range of animation frames from a CGF/TIL/LZP file.
 * Mirrors Resource.load_surfaces().
 */
export async function loadSurfaces(
  game: string,
  name: string,
  start: number,
  end: number,
): Promise<ImageBitmap[]> {
  const gfx = gfxDir(game);
  const data = await readFromDataDir([game, gfx, name]);
  const filePath = `${game}/${gfx}/${name}`;

  // For CGF files, try to pass along the sibling TIL for palette
  const ext = name.split('.').pop()?.toUpperCase() ?? '';
  const tilData = ext === 'CGF' ? await findSiblingTil(game, gfx) : null;

  const all = await decodeSurfaces(filePath, data, tilData);
  return all.slice(start, end + 1);
}

/**
 * Load a single image file as an ImageBitmap.
 * Mirrors Resource.load_surface_raw().
 */
export async function loadSurfaceRaw(game: string, name: string): Promise<ImageBitmap> {
  const data = await readFromDataDir([game, 'gfx', name]);
  const blob = new Blob([data as Uint8Array<ArrayBuffer>]);
  return createImageBitmap(blob);
}

/**
 * Load a static resource image (from game-web/resources/) via fetch.
 * Mirrors Resource.load_surface_res().
 */
export async function loadSurfaceRes(name: string): Promise<ImageBitmap> {
  const url = `${import.meta.env.BASE_URL}${name}`;
  const resp = await fetch(url);
  if (!resp.ok) throw new Error(`loadSurfaceRes: HTTP ${resp.status} for ${url}`);
  const blob = await resp.blob();
  try {
    return await createImageBitmap(blob);
  } catch (e) {
    throw new Error(`loadSurfaceRes: createImageBitmap failed for ${url} — ${e}`);
  }
}

// ─── Sync / OOS ──────────────────────────────────────────────────────────────

/**
 * Load and decode an OOS animation sync file.
 * Mirrors Resource.load_sync().
 */
export async function loadSync(game: string, filename: string): Promise<number[]> {
  const data = await readFromDataDir([game, 'Syncs', filename]);
  return decodeSyncData(`${game}/Syncs/${filename}`, data);
}

// ─── Audio path helpers ───────────────────────────────────────────────────────
// These return a string key that audioHelper uses to locate and load the file.

/**
 * Returns the path components for a voice/speak audio file.
 * Mirrors Resource.load_speak().
 */
export function loadSpeak(game: string, filename: string): string {
  const dir = game === 'RopeOutroData' ? 'speak' : 'speaks';
  return `${game}/${dir}/${filename}`;
}

/**
 * Returns the path components for a sound-effect audio file.
 * Mirrors Resource.load_sfx().
 */
export function loadSfx(game: string, filename: string): string {
  const dir = game === 'RopeOutroData' ? 'SFX' : 'sfx';
  return `${game}/${dir}/${filename}`;
}

/**
 * Read an audio file by path string.
 * Paths starting with 'music/' or 'audio_for_videos/' are served via HTTP from
 * the static resources folder.  All other paths are read from DATA_DIR (as
 * returned by loadSpeak/loadSfx).
 */
export async function readAudioFile(resourcePath: string): Promise<ArrayBuffer> {
  if (
    resourcePath.startsWith('music/') ||
    resourcePath.startsWith('audio_for_videos/') ||
    resourcePath.startsWith('sfx/')
  ) {
    const url = `${import.meta.env.BASE_URL}${resourcePath}`;
    const resp = await fetch(url);
    if (!resp.ok) throw new Error(`readAudioFile: HTTP ${resp.status} for ${url}`);
    return resp.arrayBuffer();
  }
  const parts = resourcePath.split('/');
  const data = await readFromDataDir(parts);
  return data.buffer as ArrayBuffer;
}
