/** Port of game/forest/hurt_branch_animation.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { HurtBranchTalking } from './hurtBranchTalking';
import { RenderType } from '../renderType';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class HurtBranchAnimation extends State {
  readonly needsBottom = RenderType.POST;

  constructor(context: GameData) { super(context); }

  processEvents(_events: PhoneEvents): StateClass | null {
    return this.getFrameIndexFast() >= ForestResources.hugohitlog.length ? HurtBranchTalking : null;
  }

  onEnter(): void {
    super.onEnter();
    AudioHelper.play(ForestResources.sfxBirds);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(ForestResources.hugohitlog, this.getFrameIndexFast()), 0, 0);
  }
}
