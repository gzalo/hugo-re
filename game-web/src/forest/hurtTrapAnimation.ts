/** Port of game/forest/hurt_trap_animation.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { ForestResources } from './forestResources';
import { HurtTrapTalking } from './hurtTrapTalking';
import type { PhoneEvents } from '../phoneEvents';

export class HurtTrapAnimation extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    return this.getFrameIndex() >= ForestResources.hugoTraphurt.length ? HurtTrapTalking : null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(ForestResources.hugoTraphurt, this.getFrameIndex()), 0, 0);
  }
}
