/** Port of game/cave/going_rope.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { CaveResources } from './caveResources';
import { Lost } from './lost';
import { ScyllaLost } from './scyllaLost';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class GoingRope extends State {
  private animation: ImageBitmap[];

  constructor(context: GameData) {
    super(context);
    this.animation = [CaveResources.firstRope, CaveResources.secondRope, CaveResources.thirdRope][context.caveSelectedRope];
  }

  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndex() >= this.animation.length) return this.calcWinOrLose();

    const soundTime = [2, 3, 4][this.context.caveSelectedRope];
    if (this.oneShot(soundTime, 'HivIReb')) AudioHelper.play(CaveResources.hivIReb);
    if (this.every(0.4, 'Fodtrin1'))       AudioHelper.play(CaveResources.fodtrin1);
    if (this.every(0.4, 'Fodtrin2', 0.4)) AudioHelper.play(CaveResources.fodtrin2);

    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(this.animation, this.getFrameIndex()), 0, 0);
  }

  private calcWinOrLose(): StateClass {
    const r = Math.floor(Math.random() * 4);
    if (r === 0) {
      return Lost;
    } else if (r === 1) {
      this.context.caveWinType = 0;
      this.context.forestScore *= 2;
      return ScyllaLost;
    } else if (r === 2) {
      this.context.caveWinType = 1;
      this.context.forestScore *= 3;
      return ScyllaLost;
    } else {
      this.context.caveWinType = 2;
      this.context.forestScore *= 4;
      return ScyllaLost;
    }
  }
}
