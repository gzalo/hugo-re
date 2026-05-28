/** Port of game/tv_show/attract.py */

import { type StateClass } from '../state';
import { TvShowResources } from './tvShowResources';
import { VideoState } from './videoState';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

// Deferred imports to avoid circular dependency issues at module load time
import { Initial } from './initial';
import { Playing } from './playing';
import { InCave } from './inCave';
import { InScoreboard } from './inScoreboard';

export class Attract extends VideoState {
  constructor(context: GameData) {
    super(
      context,
      TvShowResources.videosAttract[context.country],
      true,
      TvShowResources.audioAttract[context.country],
    );
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    super.render(ctx);
  }

  processEvents(events: PhoneEvents): StateClass | null {
    super.processEvents(events);

    if (events.offhook) return Initial;

    /*if (events.press_5) return Playing;
    if (events.press_4) {
      this.context.forestScore = 1000;
      this.context.forestNormalSacksCollected = 5;
      this.context.forestReachedEnd = true;
      this.context.forestGoldenSacksCollected = 1;
      this.context.forestObstaclesHit = ['1'];
      return InScoreboard;
    }
    if (events.press_6) {
      this.context.forestScore = 1000;
      return InCave;
    }*/

    return null;
  }
}
