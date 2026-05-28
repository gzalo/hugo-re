/** Port of game/cave/waiting_input.py */

import { State, type StateClass } from '../state';
import { AudioHelper } from '../audioHelper';
import { CaveResources } from './caveResources';
import { GoingRope } from './goingRope';
import { Config } from '../config';
import type { PhoneEvents } from '../phoneEvents';

export class WaitingInput extends State {
  processEvents(phoneEvents: PhoneEvents): StateClass | null {
    if (phoneEvents.press_3) { this.context.caveSelectedRope = 0; return GoingRope; }
    if (phoneEvents.press_6) { this.context.caveSelectedRope = 1; return GoingRope; }
    if (phoneEvents.press_9) { this.context.caveSelectedRope = 2; return GoingRope; }
    if (this.oneShot(0.5, 'afskylia_snak')) AudioHelper.play(CaveResources.afskyliaSnak);
    return null;
  }

  onEnter(): void {
    super.onEnter();
    if (!Config.ARGENTINE_VERSION) {
      this.context.caveStemningId = AudioHelper.play(CaveResources.stemning);
    }
  }

  onExit(): void {
    super.onExit();
    AudioHelper.play(CaveResources.tastTrykket);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(CaveResources.firstRope[0], 0, 0);
  }
}
