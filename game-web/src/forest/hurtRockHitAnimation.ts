/** Port of game/forest/hurt_rock_hit_animation.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { ForestResources } from './forestResources';
import { HurtRockTalking } from './hurtRockTalking';
import { RenderType } from '../renderType';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class HurtRockHitAnimation extends State {
  readonly needsBottom = RenderType.POST;

  constructor(context: GameData) { super(context); }

  processEvents(_events: PhoneEvents): StateClass | null {
    return this.getFrameIndexFast() >= ForestResources.hitRock.length ? HurtRockTalking : null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(ForestResources.hitRock, this.getFrameIndexFast()), 0, 0);
  }
}
