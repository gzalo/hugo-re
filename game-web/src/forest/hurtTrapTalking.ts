/** Port of game/forest/hurt_trap_talking.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { ForestUtils } from './forestUtils';
import type { PhoneEvents } from '../phoneEvents';

export class HurtTrapTalking extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndex() >= ForestResources.syncTrap.length) {
      return ForestUtils.reduceLives(this.context, 2); // Trap
    }
    return null;
  }

  onEnter(): void {
    super.onEnter();
    AudioHelper.play(ForestResources.speakTrap);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getSyncFrame(ForestResources.hugoTraptalk, ForestResources.syncTrap, this.getFrameIndex()), 0, 0);
  }
}
