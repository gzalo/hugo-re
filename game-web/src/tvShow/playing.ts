/** Port of game/tv_show/playing.py */

import { State, type StateClass } from '../state';
import { ForestGame } from '../forest/forestGame';
import { Attract } from './attract';
import { InScoreboard } from './inScoreboard';
import type { EffectType } from '../effectType';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class Playing extends State {
  private forest: ForestGame | null;

  constructor(context: GameData) {
    super(context);
    this.forest = new ForestGame(context);
  }

  processEvents(events: PhoneEvents): StateClass | null {
    if (events.hungup) return Attract;
    if (this.forest!.ended) return InScoreboard;
    this.forest!.processEvents(events);
    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    this.forest!.render(ctx);
  }

  onExit(): void {
    super.onExit();
    if (this.forest !== null && !this.forest.ended) {
      this.forest.end();
    }
    this.forest = null;
  }

  externalEffect(effect: EffectType): void {
    if (this.forest !== null) {
      this.forest.externalEffect(effect);
    }
  }
}
