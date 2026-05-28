/** Port of game/forest/playing.py */

import { State, type StateClass } from '../state';
import { AudioHelper } from '../audioHelper';
import { ForestResources } from './forestResources';
import { HurtBranchAnimation } from './hurtBranchAnimation';
import { HurtFlyingStart } from './hurtFlyingStart';
import { HurtRockAnimation } from './hurtRockAnimation';
import { HurtTrapAnimation } from './hurtTrapAnimation';
import { WinTalking } from './winTalking';
import { globalState } from '../globalState';
import { Config } from '../config';
import { EffectType } from '../effectType';
import { RenderType } from '../renderType';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class Playing extends State {
  static readonly HINT_LOOKAHEAD = 1;
  static readonly HINT_DURATION = 1.0;

  readonly needsBackground = RenderType.PRE;
  readonly needsBottom = RenderType.PRE;

  arrowUpFocus = false;
  arrowDownFocus = false;
  hugoJumpingTime: number | null = null;
  hugoCrawlingTime: number | null = null;
  lastTime: number;
  oldSecond: number | null = null;
  readonly HUGO_X_POS = 16;

  hintShownForIndex = -1;
  hintStartTime: number | null = null;
  hintType: 'dos' | 'ocho' | null = null;

  constructor(context: GameData) {
    super(context);
    this.lastTime = globalState.frameTime;
  }

  processEvents(phoneEvents: PhoneEvents): StateClass | null {
    if (!this.arrowUpFocus && !this.arrowDownFocus) {
      if (phoneEvents.press_2) {
        this.arrowUpFocus = true;
        this.hugoJumpingTime = globalState.frameTime;
      }
      if (phoneEvents.press_8) {
        this.arrowDownFocus = true;
        this.hugoCrawlingTime = globalState.frameTime;
      }
    }

    if (this.context.forestParallaxPos > Config.FOREST_MAX_TIME) {
      this.context.forestParallaxPos = Config.FOREST_MAX_TIME;
      return WinTalking;
    } else {
      this.context.forestParallaxPos += globalState.frameTime - this.lastTime;
      this.lastTime = globalState.frameTime;
    }

    const frac = this.context.forestParallaxPos % 1;
    let integer = Math.floor(this.context.forestParallaxPos - frac) + 1;
    if (integer >= Config.FOREST_MAX_TIME) integer = Config.FOREST_MAX_TIME - 1;

    if (this.oldSecond === null) {
      this.oldSecond = Math.floor(this.context.forestParallaxPos);
    }

    // Hint system
    const hintCheckIndex = integer + Playing.HINT_LOOKAHEAD;
    if (hintCheckIndex < (this.context.forestObstacles as number[]).length && hintCheckIndex !== this.hintShownForIndex) {
      const upcomingObstacle = (this.context.forestObstacles as number[])[hintCheckIndex];
      if (upcomingObstacle !== 0) {
        const needsJump = [1, 2, 3].includes(upcomingObstacle);
        const hintType: 'dos' | 'ocho' = this.context.forestControlsInverted
          ? (needsJump ? 'ocho' : 'dos')
          : (needsJump ? 'dos' : 'ocho');

        AudioHelper.play(hintType === 'dos' ? ForestResources.sfxHintDos : ForestResources.sfxHintOcho);
        this.hintShownForIndex = hintCheckIndex;
        this.hintStartTime = globalState.frameTime;
        this.hintType = hintType;
      }
    }
    if (this.hintStartTime !== null && globalState.frameTime - this.hintStartTime > Playing.HINT_DURATION) {
      this.hintType = null;
    }

    if (this.arrowUpFocus && this.hugoJumpingTime !== null && globalState.frameTime - this.hugoJumpingTime > 0.75) {
      this.hugoJumpingTime = null;
      this.arrowUpFocus = false;
    }
    if (this.arrowDownFocus && this.hugoCrawlingTime !== null && globalState.frameTime - this.hugoCrawlingTime > 0.75) {
      this.hugoCrawlingTime = null;
      this.arrowDownFocus = false;
    }

    if (this.oldSecond !== Math.floor(this.context.forestParallaxPos)) {
      const obstacles = this.context.forestObstacles as number[];
      const sacks = this.context.forestSacks as number[];

      if (obstacles[integer] !== 0 && !Config.GOD_MODE) {
        if (obstacles[integer] === 1 && !this.arrowUpFocus) {
          AudioHelper.play(ForestResources.sfxHugoLaunch);
          AudioHelper.play(ForestResources.sfxCatapultEject);
          obstacles[integer] = 0;
          return HurtFlyingStart;
        } else if (obstacles[integer] === 2 && !this.arrowUpFocus) {
          AudioHelper.play(ForestResources.sfxHugoHittrap);
          obstacles[integer] = 0;
          return HurtTrapAnimation;
        } else if (obstacles[integer] === 3 && !this.arrowUpFocus) {
          AudioHelper.play(ForestResources.sfxHugoHitlog);
          obstacles[integer] = 0;
          return HurtRockAnimation;
        } else if (obstacles[integer] === 4) {
          if (this.arrowDownFocus) {
            AudioHelper.play(ForestResources.sfxTreeSwush);
          } else {
            AudioHelper.play(ForestResources.sfxHugoHitlog);
            obstacles[integer] = 0;
            return HurtBranchAnimation;
          }
        }
      }

      if (this.arrowUpFocus && sacks[integer] !== 0) {
        if (sacks[integer] === 1) {
          this.context.forestScore += 100;
          this.context.forestNormalSacksCollected += 1;
          AudioHelper.play(ForestResources.sfxSackNormal);
          sacks[integer] = 0;
          globalState.attacks.push([this.context.country, EffectType.SPLAT, globalState.frameTime]);
        } else if (sacks[integer] === 2) {
          this.context.forestScore += 250;
          this.context.forestGoldenSacksCollected += 1;
          AudioHelper.play(ForestResources.sfxSackBonus);
          sacks[integer] = 0;
          globalState.attacks.push([this.context.country, EffectType.INVERT, globalState.frameTime]);
        }
      }
    }

    if (this.getFrameIndex() % 8 === 0 && !this.arrowUpFocus) {
      const walks = [
        ForestResources.sfxHugoWalk0, ForestResources.sfxHugoWalk1, ForestResources.sfxHugoWalk2,
        ForestResources.sfxHugoWalk3, ForestResources.sfxHugoWalk4,
      ];
      AudioHelper.play(walks[Math.floor(Math.random() * 5)]);
    }

    this.oldSecond = Math.floor(this.context.forestParallaxPos);
    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    const frac = this.context.forestParallaxPos % 1;
    const obstacles = this.context.forestObstacles as number[];
    const sacks = this.context.forestSacks as number[];
    const leaves = this.context.forestLeaves as number[];

    for (let index = 0; index < obstacles.length; index++) {
      const obstaclePos = (index - this.context.forestParallaxPos) * Config.FOREST_GROUND_SPEED;

      if (obstacles[index] === 1) {
        const idx = this.getFrameIndex() % ForestResources.catapult.length;
        const dy = [45, 43, 39, 34, 29, 22, 14, 1];
        ctx.drawImage(ForestResources.catapult[idx], obstaclePos - 8, 112 + dy[idx]);
      }
      if (obstacles[index] === 2) {
        const idx = this.getFrameIndex() % ForestResources.trap.length;
        const dy = [176, 173, 169, 165, 176, 176];
        ctx.drawImage(ForestResources.trap[idx], obstaclePos - 8, dy[idx] - 24);
      }
      if (obstacles[index] === 3) {
        const idx = this.getFrameIndex() % ForestResources.rock.length;
        ctx.drawImage(ForestResources.rock[idx], obstaclePos - Math.sin(frac * 2 * Math.PI) * 15, 120);
      }
      if (obstacles[index] === 4) {
        const idx = this.getFrameIndex() % ForestResources.tree.length;
        ctx.drawImage(ForestResources.loneTree[0], obstaclePos - 52, -40);
        ctx.drawImage(ForestResources.tree[idx], obstaclePos, 52 + 10);
      }
    }

    for (let index = 0; index < sacks.length; index++) {
      const sackPos = (index - this.context.forestParallaxPos) * Config.FOREST_GROUND_SPEED;
      if (sacks[index] !== 0) {
        ctx.drawImage(ForestResources.sack[0], sackPos - 16, 32);
      }
    }

    for (let index = 0; index < leaves.length; index++) {
      const leavePos = (index - this.context.forestParallaxPos) * Config.FOREST_GROUND_SPEED;
      if (leaves[index] === 1) ctx.drawImage(ForestResources.leaves2[0], leavePos - 16, -10);
      else if (leaves[index] === 2) ctx.drawImage(ForestResources.leaves1[0], leavePos - 16, -10);
    }

    if (this.arrowUpFocus) {
      ctx.drawImage(ForestResources.arrows[1], 256, 17);
    } else {
      ctx.drawImage(ForestResources.arrows[0], 256 + 2, 16 + 3);
    }
    if (this.arrowDownFocus) {
      ctx.drawImage(ForestResources.arrows[3], 256, 54);
    } else {
      ctx.drawImage(ForestResources.arrows[2], 256 + 2, 54 + 2);
    }

    if (this.arrowUpFocus && this.hugoJumpingTime !== null) {
      const dt = (globalState.frameTime - this.hugoJumpingTime) / 0.75;
      const dy = -250 * dt ** 2 + 250 * dt - 22.5;
      ctx.drawImage(ForestResources.hugoJump[this.getFrameIndex() % ForestResources.hugoJump.length], this.HUGO_X_POS, 40 - dy);
    } else if (this.arrowDownFocus) {
      ctx.drawImage(ForestResources.hugoCrawl[this.getFrameIndex() % ForestResources.hugoCrawl.length], this.HUGO_X_POS, 105);
    } else {
      ctx.drawImage(ForestResources.hugoSide[this.getFrameIndex() % ForestResources.hugoSide.length], this.HUGO_X_POS, 90);
    }

    if (this.hintType) {
      ctx.drawImage(this.hintType === 'dos' ? ForestResources.hintOverlayDos! : ForestResources.hintOverlayOcho!, 0, 0);
    }
  }
}
