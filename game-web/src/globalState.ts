import type { EffectType } from './effectType';

// Attack tuple: [sourceCountry, effectType, startTime]
export type Attack = [string, EffectType, number];

export const globalState = {
  frameTime: 0,         // seconds, updated each frame
  attacks: [] as Attack[],
  anyPlaying: false,
};
