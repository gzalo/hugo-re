/** Port of game/tv_show/have_luck.py */

import { type StateClass } from '../state';
import { TvShowResources } from './tvShowResources';
import { VideoState } from './videoState';
import { Instructions } from './instructions';
import { Attract } from './attract';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class HaveLuck extends VideoState {
  constructor(context: GameData) {
    super(
      context,
      TvShowResources.videosHaveLuck[context.country],
      false,
      TvShowResources.audioHaveLuck[context.country],
    );
  }

  processEvents(events: PhoneEvents): StateClass | null {
    super.processEvents(events);
    if (events.hungup)     return Attract;
    if (this.videoEnded()) return Instructions;
    return null;
  }
}
