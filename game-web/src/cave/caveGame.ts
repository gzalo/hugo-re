/** Port of game/cave/cave_game.py */

import { AudioHelper } from '../audioHelper';
import { CaveResources } from './caveResources';
import { WaitingBeforeTalking } from './waitingBeforeTalking';
import { NullState } from '../nullState';
import { globalState } from '../globalState';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';
import type { State } from '../state';

export class CaveGame {
  ended = false;

  private rollingScore: number;
  private context: GameData;
  private soundingScore = false;
  private caveScoreCounterStart = 0;
  private state: State;

  constructor(context: GameData) {
    this.context = context;
    this.rollingScore = context.forestScore;
    this.state = new WaitingBeforeTalking(context);
    this.state.onEnter();
  }

  processEvents(phoneEvents: PhoneEvents): void {
    const nextStateClass = this.state.processEvents(phoneEvents);

    if (nextStateClass !== null) {
      this.state.onExit();
      if (nextStateClass === NullState) {
        this.end();
      } else {
        this.state = new nextStateClass(this.context);
        this.state.onEnter();
      }
    }

    if (this.context.forestScore !== this.rollingScore && !this.soundingScore) {
      this.context.caveScoreCounterId = AudioHelper.play(CaveResources.scoreCounter);
      this.caveScoreCounterStart = globalState.frameTime;
      this.soundingScore = true;
    } else if (this.soundingScore && globalState.frameTime - this.caveScoreCounterStart >= 0.1) {
      this.context.caveScoreCounterId = AudioHelper.play(CaveResources.scoreCounter);
      this.caveScoreCounterStart = globalState.frameTime;
    }
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    this.state.render(ctx);

    if (this.rollingScore < this.context.forestScore) {
      this.rollingScore += Math.min(10, this.context.forestScore - this.rollingScore);
    } else if (this.soundingScore) {
      AudioHelper.stop(this.context.caveScoreCounterId);
      this.soundingScore = false;
    }

    this.renderScore(ctx);
  }

  private renderScore(ctx: OffscreenCanvasRenderingContext2D): void {
    const xScoreRight = 210 + 16 * 6;  // right edge position (6 digits max)
    const yScore = 203;
    const xSpace = 16;

    const score = Math.floor(this.rollingScore);
    if (score === 0) {
      ctx.drawImage(CaveResources.scoreFont[0], xScoreRight - xSpace, yScore);
      return;
    }

    const digits: number[] = [];
    let temp = score;
    while (temp > 0) {
      digits.push(temp % 10);
      temp = Math.floor(temp / 10);
    }

    for (let i = 0; i < digits.length; i++) {
      ctx.drawImage(CaveResources.scoreFont[digits[i]], xScoreRight - xSpace * (i + 1), yScore);
    }
  }

  private end(): void {
    this.state.onExit();
    if (this.soundingScore) {
      AudioHelper.stop(this.context.caveScoreCounterId);
      this.soundingScore = false;
    }
    this.ended = true;
  }
}
