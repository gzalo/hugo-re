/** Port of game/cave/climbing.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { CaveResources } from './caveResources';
import { WaitingInput } from './waitingInput';
import type { PhoneEvents } from '../phoneEvents';

export class Climbing extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndex() >= CaveResources.climbs.length) return WaitingInput;
    if (this.oneShot(1, 'NuKommerJeg')) AudioHelper.play(CaveResources.nuKommerJeg);
    if (this.oneShot(2, 'PreFanfare'))  AudioHelper.play(CaveResources.preFanfare);
    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(CaveResources.climbs, this.getFrameIndex()), 0, 0);
  }
}
