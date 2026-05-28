/** Port of game/cave/lost.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { CaveResources } from './caveResources';
import { LostSpring } from './lostSpring';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class Lost extends State {
  private animation: ImageBitmap[];

  constructor(context: GameData) {
    super(context);
    this.animation = [
      CaveResources.hugoPuffFirst,
      CaveResources.hugoPuffSecond,
      CaveResources.hugoPuffThird,
    ][context.caveSelectedRope];
  }

  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndex() >= this.animation.length) return LostSpring;

    if (this.oneShot(1, 'PrePuf')) AudioHelper.play(CaveResources.prePuf);
    if (this.oneShot(2, 'Puf'))    AudioHelper.play(CaveResources.puf);

    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(this.animation, this.getFrameIndex()), 0, 0);
  }
}
