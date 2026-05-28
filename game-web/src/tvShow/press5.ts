/** Port of game/tv_show/press_5.py */

import { type StateClass } from '../state';
import { TvShowResources } from './tvShowResources';
import { VideoState } from './videoState';
import { Attract } from './attract';
import { HaveLuck } from './haveLuck';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class Press5 extends VideoState {
  constructor(context: GameData) {
    super(
      context,
      TvShowResources.videosPress5[context.country],
      true,
      TvShowResources.audioPress5[context.country],
    );
  }

  processEvents(events: PhoneEvents): StateClass | null {
    super.processEvents(events);
    if (events.hungup) return Attract;
    if (events.press_5) return HaveLuck;
    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    super.render(ctx);
    if (this.hasLooped) {
      ctx.drawImage(TvShowResources.press5!, 0, 0);
    }
  }
}
