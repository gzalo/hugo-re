/** Port of game/tv_show/going_cave.py */

import { type StateClass } from '../state';
import { TvShowResources } from './tvShowResources';
import { VideoState } from './videoState';
import { Attract } from './attract';
import { InCave } from './inCave';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class GoingCave extends VideoState {
  constructor(context: GameData) {
    super(
      context,
      TvShowResources.videosGoingScylla[context.country],
      false,
      TvShowResources.audioGoingScylla[context.country],
    );
  }

  processEvents(events: PhoneEvents): StateClass | null {
    super.processEvents(events);
    if (events.hungup)     return Attract;
    if (this.videoEnded()) return InCave;
    return null;
  }
}
