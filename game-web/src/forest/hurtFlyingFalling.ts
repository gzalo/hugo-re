/** Port of game/forest/hurt_flying_falling.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { HurtFlyingFallingHangAnimation } from './hurtFlyingFallingHangAnimation';
import type { PhoneEvents } from '../phoneEvents';

export class HurtFlyingFalling extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    return this.getFrameIndexFast() >= ForestResources.catapultFall.length ? HurtFlyingFallingHangAnimation : null;
  }

  onEnter(): void {
    super.onEnter();
    AudioHelper.play(ForestResources.speakCatapultDown);
    AudioHelper.play(ForestResources.sfxHugoCrash);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(ForestResources.catapultFall, this.getFrameIndexFast()), 0, 0);
  }
}
