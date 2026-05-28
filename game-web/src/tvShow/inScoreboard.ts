/** Port of game/tv_show/in_scoreboard.py */

import { State, type StateClass } from '../state';
import { ScoreboardGame } from '../scoreboard/scoreboardGame';
import { Attract } from './attract';
import { GoingCave } from './goingCave';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class InScoreboard extends State {
  private scoreboard: ScoreboardGame | null;

  constructor(context: GameData) {
    super(context);
    this.scoreboard = new ScoreboardGame(context);
  }

  processEvents(events: PhoneEvents): StateClass | null {
    if (events.hungup) return Attract;
    if (this.scoreboard!.ended) return GoingCave;
    this.scoreboard!.processEvents(events);
    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    this.scoreboard!.render(ctx);
  }

  onExit(): void {
    super.onExit();
    if (this.scoreboard !== null && !this.scoreboard.ended) {
      this.scoreboard.end();
    }
    this.scoreboard = null;
  }
}
