/** Port of game/tv_show/tv_show_parent.py */

import { Config } from '../config';
import { Attract } from './attract';
import { Playing } from './playing';
import type { EffectType } from '../effectType';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';
import type { State } from '../state';

export class TvShowParent {
  readonly country: string;
  currentGame: string;

  private context: GameData;
  private state: State;

  constructor(context: GameData) {
    this.context = context;
    this.country = context.country;
    this.currentGame = Object.keys(Config.GAMES)[0];

    this.state = new Attract(context);
    this.state.onEnter();
  }

  setRandomGame(): void {
    const games = Object.keys(Config.GAMES);
    if (games.length === 1) {
      this.currentGame = games[0];
      return;
    }
    let newGame: string;
    do {
      newGame = games[Math.floor(Math.random() * games.length)];
    } while (newGame === this.currentGame);
    this.currentGame = newGame;
  }

  handleEvents(phoneEvents: PhoneEvents): void {
    const nextStateClass = this.state.processEvents(phoneEvents);
    if (nextStateClass !== null) {
      this.state.onExit();
      this.state = new nextStateClass(this.context);
      this.state.onEnter();
    }
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    this.state.render(ctx);
  }

  isPlaying(): boolean {
    return !(this.state instanceof Attract);
  }

  externalEffect(effect: EffectType): void {
    if (this.state instanceof Playing) {
      this.state.externalEffect(effect);
    }
  }

  cleanup(): void {
    this.state.onExit();
  }
}
