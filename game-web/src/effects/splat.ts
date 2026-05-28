/** Port of game/effects/splat.py */

import { Config } from '../config';
import { Tween } from '../tween';

async function fetchImage(url: string): Promise<ImageBitmap> {
  const resp = await fetch(url);
  if (!resp.ok) throw new Error(`Splat: HTTP ${resp.status} for ${url}`);
  const blob = await resp.blob();
  try {
    return await createImageBitmap(blob);
  } catch (e) {
    throw new Error(`Splat: createImageBitmap failed for ${url} — ${e}`);
  }
}

export class Splat {
  static splats: ImageBitmap[] = [];
  static orb: ImageBitmap | null = null;

  static async init(): Promise<void> {
    const base = import.meta.env.BASE_URL;
    const splatPromises = Array.from({ length: 10 }, (_, i) =>
      fetchImage(`${base}splats/splat0${i}.png`),
    );
    Splat.splats = await Promise.all(splatPromises);
    Splat.orb = await fetchImage(`${base}orbs/orb.png`);
  }

  static renderSplat(ctx: OffscreenCanvasRenderingContext2D, time: number): void {
    const surface = Splat.splats[Math.floor(Math.random() * 10)];

    let alpha = 1;
    if (time > Config.EFFECT_DURATION_SPLAT / 2) {
      alpha = Tween.map(time, Config.EFFECT_DURATION_SPLAT / 2, Config.EFFECT_DURATION_SPLAT, 1, 0);
      alpha = Math.max(0, Math.min(1, alpha));
    }

    ctx.save();
    ctx.globalAlpha = alpha;
    ctx.drawImage(surface, 0, 0);
    ctx.restore();
  }
}
