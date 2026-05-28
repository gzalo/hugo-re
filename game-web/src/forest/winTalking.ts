/** Port of game/forest/win_talking.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { NullState } from '../nullState';
import { RenderType } from '../renderType';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class WinTalking extends State {
  readonly needsBackground = RenderType.PRE;
  readonly needsBottom = RenderType.PRE;

  constructor(context: GameData) { super(context); }

  processEvents(_events: PhoneEvents): StateClass | null {
    return this.getFrameIndex() >= ForestResources.syncLevelcompleted.length ? NullState : null;
  }

  onEnter(): void {
    super.onEnter();
    this.context.forestReachedEnd = true;
    AudioHelper.play(ForestResources.speakLevelcompleted);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getSyncFrame(ForestResources.hugoTelllives, ForestResources.syncLevelcompleted, this.getFrameIndex()), 128, -16);
  }
}
