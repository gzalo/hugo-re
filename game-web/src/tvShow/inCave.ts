/** Port of game/tv_show/in_cave.py */

import { State, type StateClass } from '../state';
import { CaveGame } from '../cave/caveGame';
import { Attract } from './attract';
import { Ending } from './ending';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class InCave extends State {
  private cave: CaveGame | null;

  constructor(context: GameData) {
    super(context);
    this.cave = new CaveGame(context);
  }

  processEvents(events: PhoneEvents): StateClass | null {
    if (events.hungup) return Attract;
    if (this.cave!.ended) return Ending;
    this.cave!.processEvents(events);
    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    this.cave!.render(ctx);
  }

  onExit(): void {
    super.onExit();
    this.cave = null;
  }
}
