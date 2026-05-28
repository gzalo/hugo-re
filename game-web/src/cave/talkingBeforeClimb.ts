/** Port of game/cave/talking_before_climb.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { CaveResources } from './caveResources';
import { Climbing } from './climbing';
import { Config } from '../config';
import type { PhoneEvents } from '../phoneEvents';

export class TalkingBeforeClimb extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    if (!Config.ARGENTINE_VERSION && this.oneShot(4, 'TrappeGrin')) {
      AudioHelper.play(CaveResources.trappeGrin);
    }
    if (this.getFrameIndex() >= CaveResources.syncHugoStart.length) return Climbing;
    return null;
  }

  onEnter(): void {
    super.onEnter();
    AudioHelper.play(CaveResources.trappeSnak);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getSyncFrame(CaveResources.talks, CaveResources.syncHugoStart, this.getFrameIndex()), 0, 0);
  }
}
