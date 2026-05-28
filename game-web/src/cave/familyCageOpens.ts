/** Port of game/cave/family_cage_opens.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { CaveResources } from './caveResources';
import { FamilyHappy } from './familyHappy';
import { Config } from '../config';
import type { PhoneEvents } from '../phoneEvents';

export class FamilyCageOpens extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndex() >= CaveResources.familyCage.length) return FamilyHappy;

    if (this.oneShot(0.5, 'HivIReb'))     AudioHelper.play(CaveResources.hivIReb);
    if (this.oneShot(1, 'HugolineTak'))   AudioHelper.play(CaveResources.hugolineTak);

    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(CaveResources.familyCage, this.getFrameIndex()), 0, 0);
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
