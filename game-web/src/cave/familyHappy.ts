/** Port of game/cave/family_happy.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { CaveResources } from './caveResources';
import { NullState } from '../nullState';
import type { PhoneEvents } from '../phoneEvents';

export class FamilyHappy extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndex() >= CaveResources.happy.length) return NullState;
    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(CaveResources.happy, this.getFrameIndex()), 0, 0);
  }

  onEnter(): void {
    super.onEnter();
    this.context.caveFanfareId = AudioHelper.play(CaveResources.fanfare);
  }

  onExit(): void {
    super.onExit();
    AudioHelper.stop(this.context.caveFanfareId);
  }
}
