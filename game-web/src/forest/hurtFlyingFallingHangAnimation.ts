/** Port of game/forest/hurt_flying_falling_hang_animation.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { HurtFlyingFallingHangTalking } from './hurtFlyingFallingHangTalking';
import type { PhoneEvents } from '../phoneEvents';

export class HurtFlyingFallingHangAnimation extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    return this.getFrameIndexFast() >= ForestResources.catapultHang.length ? HurtFlyingFallingHangTalking : null;
  }

  onEnter(): void {
    super.onEnter();
    AudioHelper.play(ForestResources.sfxHugoHangstart);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(ForestResources.catapultHang, this.getFrameIndexFast()), 0, 0);
  }
}
