/** Port of game/cave/lost_spring.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { CaveResources } from './caveResources';
import { NullState } from '../nullState';
import { Config } from '../config';
import type { PhoneEvents } from '../phoneEvents';

export class LostSpring extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndex() >= CaveResources.hugoSpring.length) return NullState;

    if (this.oneShot(2.5, 'HugoSkydUd')) AudioHelper.play(CaveResources.hugoSkydUd);

    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(CaveResources.hugoSpring, this.getFrameIndex()), 0, 0);
  }

  onEnter(): void {
    super.onEnter();
    AudioHelper.play(CaveResources.hugoKatapult);
  }

  onExit(): void {
    super.onExit();
    if (Config.ARGENTINE_VERSION) {
      AudioHelper.fadeout(this.context.caveBgMusicId, 1000);
    } else {
      AudioHelper.fadeout(this.context.caveStemningId, 1000);
    }
  }
}
