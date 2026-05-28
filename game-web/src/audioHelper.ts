/**
 * Port of game/audio_helper.py.
 * Replaces UDP audio server with Web Audio API.
 * play() returns an ID synchronously (like Python's UDP version) and starts
 * audio asynchronously in the background from a pre-loaded cache.
 */

import { readAudioFile } from './resource';

let _audioCtx: AudioContext | null = null;

function getAudioCtx(): AudioContext {
  if (!_audioCtx) _audioCtx = new AudioContext();
  return _audioCtx;
}

// ─── Buffer cache ─────────────────────────────────────────────────────────────

const _bufferCache = new Map<string, AudioBuffer>();

/** Pre-load an audio resource into the cache. Call during init. */
export async function preloadAudio(resource: string): Promise<void> {
  if (_bufferCache.has(resource)) return;
  try {
    const ctx = getAudioCtx();
    const raw = await readAudioFile(resource);
    const buf = await ctx.decodeAudioData(raw);
    _bufferCache.set(resource, buf);
  } catch (e) {
    console.warn(`preloadAudio failed for ${resource}:`, e);
  }
}

// ─── Instance tracking ────────────────────────────────────────────────────────

interface Instance {
  source: AudioBufferSourceNode | null;
  gain: GainNode | null;
  active: boolean;
}

let _nextId = 1;
const _instances = new Map<number, Instance>();

async function _loadAndPlay(id: number, resource: string, loops: number): Promise<void> {
  try {
    const ctx = getAudioCtx();
    if (ctx.state === 'suspended') await ctx.resume();
    let buffer = _bufferCache.get(resource);
    if (!buffer) {
      const raw = await readAudioFile(resource);
      buffer = await ctx.decodeAudioData(raw);
      _bufferCache.set(resource, buffer);
    }

    const inst = _instances.get(id);
    if (!inst || !inst.active) return; // was stopped before we loaded

    const source = ctx.createBufferSource();
    source.buffer = buffer;
    source.loop = (loops === -1);

    const gain = ctx.createGain();
    gain.gain.value = 1.0;
    source.connect(gain);
    gain.connect(ctx.destination);

    inst.source = source;
    inst.gain = gain;

    source.onended = () => { _instances.delete(id); };
    source.start(0);
  } catch (e) {
    console.warn(`AudioHelper._loadAndPlay failed for ${resource}:`, e);
    _instances.delete(id);
  }
}

// ─── Public API ───────────────────────────────────────────────────────────────

export const AudioHelper = {
  /**
   * Play a sound. Returns an instance ID synchronously (like Python UDP version).
   * Audio starts playing async from the pre-loaded cache.
   * loops: 0 = play once, -1 = loop forever.
   */
  play(resource: string, loops: number = 0): number {
    const id = _nextId++;
    _instances.set(id, { source: null, gain: null, active: true });
    void _loadAndPlay(id, resource, loops);
    return id;
  },

  /** Stop a sound immediately. */
  stop(instanceId: number | null): void {
    if (instanceId === null) return;
    const inst = _instances.get(instanceId);
    if (!inst) return;
    inst.active = false;
    if (inst.source) {
      try { inst.source.stop(); } catch { /* already stopped */ }
    }
    _instances.delete(instanceId);
  },

  /** Fade out a sound over `ms` milliseconds. */
  fadeout(instanceId: number | null, ms: number = 0): void {
    if (instanceId === null) return;
    const inst = _instances.get(instanceId);
    if (!inst) return;
    if (ms <= 0 || !inst.gain) {
      AudioHelper.stop(instanceId);
      return;
    }
    const ctx = getAudioCtx();
    const secs = ms / 1000;
    inst.gain.gain.setValueAtTime(inst.gain.gain.value, ctx.currentTime);
    inst.gain.gain.linearRampToValueAtTime(0, ctx.currentTime + secs);
    setTimeout(() => AudioHelper.stop(instanceId), ms + 50);
  },
};
