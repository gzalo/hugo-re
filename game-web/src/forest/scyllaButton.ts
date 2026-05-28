/** Port of game/forest/scylla_button.py */

import { State, type StateClass } from '../state';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { Playing } from './playing';
import type { PhoneEvents } from '../phoneEvents';

export class ScyllaButton extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.oneShot(0.5, 'sfx_lightning_warning')) {
      AudioHelper.play(ForestResources.sfxLightningWarning);
    }
    if (this.getStateTime() > 2) return Playing;
    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    const images = [ForestResources.scullaHand1[0], ForestResources.scullaHand2[0]];
    ctx.drawImage(images[this.getFrameIndex() % 2], 0, 0);
  }
}
