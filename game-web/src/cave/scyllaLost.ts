/** Port of game/cave/scylla_lost.py */

import { State, type StateClass } from '../state';
import { Animation } from '../animation';
import { AudioHelper } from '../audioHelper';
import { CaveResources } from './caveResources';
import { FamilyCageOpens } from './familyCageOpens';
import { ScyllaSpring } from './scyllaSpring';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class ScyllaLost extends State {
  private static readonly hugoSpritePos: [number, number][] = [
    [25, 105],
    [97, 100],
    [172, 102],
  ];

  private animation: ImageBitmap[];

  constructor(context: GameData) {
    super(context);
    this.animation = [
      CaveResources.scyllaBird,
      CaveResources.scyllaLeaves,
      CaveResources.scyllaRopes,
    ][context.caveWinType];
  }

  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.getFrameIndex() >= this.animation.length) {
      return this.context.caveWinType === 2 ? ScyllaSpring : FamilyCageOpens;
    }

    if (this.context.caveWinType === 0) {
      if (this.oneShot(0.5, 'FugleSkrig')) AudioHelper.play(CaveResources.fugleSkrig);
    } else if (this.context.caveWinType === 1) {
      if (this.oneShot(0.5, 'Skrig')) AudioHelper.play(CaveResources.skrig);
    } else {
      if (this.oneShot(1, 'PrePuf')) AudioHelper.play(CaveResources.prePuf);
      if (this.oneShot(2, 'Puf'))    AudioHelper.play(CaveResources.puf);
    }

    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(Animation.getFrame(this.animation, this.getFrameIndex()), 0, 0);
    const [hx, hy] = ScyllaLost.hugoSpritePos[this.context.caveSelectedRope];
    ctx.drawImage(CaveResources.hugo[0], hx, hy);
  }
}
