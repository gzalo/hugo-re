/** Port of game/tv_show/ending.py */

import { type StateClass } from '../state';
import { TvShowResources } from './tvShowResources';
import { VideoState } from './videoState';
import { Attract } from './attract';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class Ending extends VideoState {
  constructor(context: GameData) {
    super(
      context,
      TvShowResources.videosEnding[context.country],
      false,
      TvShowResources.audioEnding[context.country],
    );
  }

  processEvents(events: PhoneEvents): StateClass | null {
    super.processEvents(events);
    if (events.hungup) return Attract;
    return null;
  }
}
