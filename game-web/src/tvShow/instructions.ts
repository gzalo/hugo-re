/** Port of game/tv_show/instructions.py */

import { State, type StateClass } from '../state';
import { Config } from '../config';
import { TvShowResources } from './tvShowResources';
import { Attract } from './attract';
import { Playing } from './playing';
import type { PhoneEvents } from '../phoneEvents';

export class Instructions extends State {
  processEvents(events: PhoneEvents): StateClass | null {
    if (events.hungup) return Attract;
    if (this.getStateTime() > Config.INSTRUCTIONS_TIMEOUT) return Playing;
    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(TvShowResources.instructions['Forest'], 0, 0);
  }
}
