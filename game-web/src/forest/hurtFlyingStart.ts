/** Port of game/forest/hurt_flying_start.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { HurtFlyingTalking } from './hurtFlyingTalking';
import type { PhoneEvents } from '../phoneEvents';

export class HurtFlyingStart extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndexFast() >= ForestResources.catapultFly.length) return HurtFlyingTalking;
    if (this.oneShot(2.7, 'crash')) AudioHelper.play(ForestResources.sfxHugoScreenklir);
    if (this.oneShot(2.7, 'uy'))   AudioHelper.play(ForestResources.speakCatapultHit);
    return null;
  }

  onEnter(): void {
    super.onEnter();
    AudioHelper.play(ForestResources.speakCatapultUp);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(ForestResources.catapultFly, this.getFrameIndexFast()), 0, 0);
  }
}
