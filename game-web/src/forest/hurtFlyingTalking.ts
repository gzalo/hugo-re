/** Port of game/forest/hurt_flying_talking.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { HurtFlyingFalling } from './hurtFlyingFalling';
import type { PhoneEvents } from '../phoneEvents';

export class HurtFlyingTalking extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    return this.getFrameIndex() >= ForestResources.syncCatapultTalktop.length ? HurtFlyingFalling : null;
  }

  onEnter(): void {
    super.onEnter();
    AudioHelper.play(ForestResources.speakCatapultTalktop);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getSyncFrame(ForestResources.catapultAirtalk, ForestResources.syncCatapultTalktop, this.getFrameIndex()), 0, 0);
  }
}
