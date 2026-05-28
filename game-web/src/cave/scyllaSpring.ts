/** Port of game/cave/scylla_spring.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { CaveResources } from './caveResources';
import { FamilyCageOpens } from './familyCageOpens';
import type { PhoneEvents } from '../phoneEvents';

export class ScyllaSpring extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndex() >= CaveResources.scyllaSpring.length) return FamilyCageOpens;

    if (this.oneShot(1.5, 'Fjeder'))        AudioHelper.play(CaveResources.fjeder);
    if (this.oneShot(2, 'AfskyliaSkydUd')) AudioHelper.play(CaveResources.afskyliaSkyudUd);

    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(CaveResources.scyllaSpring, this.getFrameIndex()), 0, 0);
  }
}
