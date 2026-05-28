/** Port of game/tv_show/initial.py */

import { type StateClass } from '../state';
import { TvShowResources } from './tvShowResources';
import { VideoState } from './videoState';
import { Press5 } from './press5';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

// Deferred to avoid circular reference with Attract
import { Attract } from './attract';

export class Initial extends VideoState {
  constructor(context: GameData) {
    super(
      context,
      TvShowResources.videosInitial[context.country],
      false,
      TvShowResources.audioInitial[context.country],
    );
  }

  processEvents(events: PhoneEvents): StateClass | null {
    super.processEvents(events);

    if (events.hungup)     return Attract;
    if (this.videoEnded()) return Press5;

    return null;
  }
}
