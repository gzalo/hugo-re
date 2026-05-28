/** Port of game/forest/hurt_flying_falling_hang_talking.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { ForestUtils } from './forestUtils';
import type { PhoneEvents } from '../phoneEvents';

export class HurtFlyingFallingHangTalking extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndex() >= ForestResources.syncCatapultHang.length) {
      return ForestUtils.reduceLives(this.context, 1); // Catapult
    }
    return null;
  }

  onEnter(): void {
    super.onEnter();
    AudioHelper.play(ForestResources.speakCatapultHang);
    AudioHelper.play(ForestResources.sfxHugoHang);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(ForestResources.catapultHang[12], 0, 0);
    ctx.drawImage(Animation.getSyncFrame(ForestResources.catapultHangspeak, ForestResources.syncCatapultHang, this.getFrameIndex()), 115, 117);
  }
}
