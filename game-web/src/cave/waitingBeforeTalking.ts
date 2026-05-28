/** Port of game/cave/waiting_before_talking.py */

import { State, type StateClass } from '../state';
import { AudioHelper } from '../audioHelper';
import { CaveResources } from './caveResources';
import { TalkingBeforeClimb } from './talkingBeforeClimb';
import { Config } from '../config';
import type { PhoneEvents } from '../phoneEvents';

export class WaitingBeforeTalking extends State {
  processEvents(_events: PhoneEvents): StateClass | null {
    return this.getStateTime() > 2.5 ? TalkingBeforeClimb : null;
  }

  onEnter(): void {
    super.onEnter();
    AudioHelper.play(CaveResources.herErVi);
    if (Config.ARGENTINE_VERSION) {
      this.context.caveBgMusicId = AudioHelper.play(CaveResources.bgMusic);
    } else {
      this.context.caveStemningId = AudioHelper.play(CaveResources.stemning);
    }
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(CaveResources.talks[12], 0, 0);
  }
}
