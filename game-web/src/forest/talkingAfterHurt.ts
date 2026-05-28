/** Port of game/forest/talking_after_hurt.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { Playing } from './playing';
import { RenderType } from '../renderType';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class TalkingAfterHurt extends State {
  readonly needsBackground = RenderType.PRE;
  readonly needsBottom = RenderType.PRE;

  constructor(context: GameData) { super(context); }

  processEvents(_events: PhoneEvents): StateClass | null {
    if (
      (this.context.forestLives === 1 && this.getFrameIndex() >= ForestResources.syncLastlife.length) ||
      (this.context.forestLives >= 1 && this.getFrameIndex() >= ForestResources.syncDieonce.length)
    ) {
      return Playing;
    }

    if (this.oneShot(0.5, 'knock1')) AudioHelper.play(ForestResources.sfxHugoKnock);
    if (this.oneShot(0, 'speak')) {
      AudioHelper.play(ForestResources.sfxHugoKnock);
      if (this.context.forestLives === 1) {
        AudioHelper.play(ForestResources.speakLastlife);
      } else {
        AudioHelper.play(ForestResources.speakDieonce);
      }
    }

    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    const sync = this.context.forestLives === 1 ? ForestResources.syncLastlife : ForestResources.syncDieonce;
    ctx.drawImage(Animation.getSyncFrame(ForestResources.hugoTelllives, sync, this.getFrameIndex()), 128, -16);

    if (this.getFrameIndex() < 8 && this.getFrameIndex() % 4 === 0) {
      ctx.drawImage(ForestResources.hugoHand2[0], 96, 78);
    } else {
      ctx.drawImage(ForestResources.hugoHand1[0], 96, 83);
    }
  }
}
