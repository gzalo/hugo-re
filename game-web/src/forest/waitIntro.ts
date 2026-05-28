/** Port of game/forest/wait_intro.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { Playing } from './playing';
import { RenderType } from '../renderType';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class WaitIntro extends State {
  readonly needsBackground = RenderType.PRE;
  readonly needsBottom = RenderType.PRE;

  constructor(context: GameData) { super(context); }

  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndex() >= ForestResources.syncStart.length) {
      return Playing;
    }
    return null;
  }

  onEnter(): void {
    super.onEnter();
    AudioHelper.play(ForestResources.speakStart);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getSyncFrame(ForestResources.hugoTelllives, ForestResources.syncStart, this.getFrameIndex()), 128, -16);
  }
}
