/** Port of game/forest/hurt_branch_talking.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { ForestUtils } from './forestUtils';
import { RenderType } from '../renderType';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class HurtBranchTalking extends State {
  readonly needsBottom = RenderType.POST;

  constructor(context: GameData) { super(context); }

  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndex() >= ForestResources.syncHitlog.length) {
      return ForestUtils.reduceLives(this.context, 4); // Branch
    }
    return null;
  }

  onEnter(): void {
    super.onEnter();
    AudioHelper.play(ForestResources.speakHitlog);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getSyncFrame(ForestResources.hugohitlogTalk, ForestResources.syncHitlog, this.getFrameIndex()), 0, 0);
  }
}
