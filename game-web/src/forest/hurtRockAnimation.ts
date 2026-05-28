/** Port of game/forest/hurt_rock_animation.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { ForestResources } from './forestResources';
import { HurtRockHitAnimation } from './hurtRockHitAnimation';
import { RenderType } from '../renderType';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class HurtRockAnimation extends State {
  readonly needsBottom = RenderType.POST;

  constructor(context: GameData) { super(context); }

  processEvents(_events: PhoneEvents): StateClass | null {
    return this.getFrameIndexFast() >= ForestResources.hugoLookrock.length ? HurtRockHitAnimation : null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(ForestResources.hugoLookrock, this.getFrameIndexFast()), 0, 0);
  }
}
