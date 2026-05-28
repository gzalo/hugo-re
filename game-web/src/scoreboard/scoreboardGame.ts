/** Port of game/scoreboard/scoreboard_game.py */

import { AudioHelper } from '../audioHelper';
import { ScoreboardResources } from './scoreboardResources';
import { globalState } from '../globalState';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class ScoreboardGame {
  ended = false;

  // Timing constants
  private static readonly HUGO_ANIM_FPS    = 20;
  private static readonly ROLL_SPEED       = 50;

  // Score values
  private static readonly SACK_SCORE       = 100;
  private static readonly GOLDEN_SACK_SCORE = 250;
  private static readonly END_BONUS        = 1500;
  private static readonly LIFE_LOST_PENALTY = 100;

  // Layout constants
  private static readonly SCORE_X          = 192;
  private static readonly SCORE_DIGIT_SPACING = 16;
  private static readonly ICON_X           = 100;
  private static readonly ROW_HEIGHT       = 49;
  private static readonly FIRST_ROW_Y      = 3;
  private static readonly HUGO_X           = 0;
  private static readonly HUGO_START_Y     = 0;
  private static readonly HUGO_END_Y       = 180;

  private context: GameData;

  private sackScore: number;
  private endBonus: number;
  private goldenScore: number;
  private livesPenalty: number;
  private totalScore: number;

  private rollingSack   = 0;
  private rollingEnd    = 0;
  private rollingGolden = 0;
  private rollingPenalty = 0;
  private rollingTotal  = 0;

  private currentRow     = 0;
  private rowPhase       = 0;  // 0=forward_anim, 1=rolling, 2=reverse_anim
  private phaseStartTime = 0;

  private isRolling        = false;
  private scoreCounterStart = 0;

  constructor(context: GameData) {
    this.context = context;

    this.sackScore    = context.forestNormalSacksCollected * ScoreboardGame.SACK_SCORE;
    this.endBonus     = context.forestReachedEnd ? ScoreboardGame.END_BONUS : 0;
    this.goldenScore  = context.forestGoldenSacksCollected * ScoreboardGame.GOLDEN_SACK_SCORE;

    const livesLost   = 3 - context.forestLives;
    this.livesPenalty = livesLost * ScoreboardGame.LIFE_LOST_PENALTY;

    this.totalScore   = this.sackScore + this.endBonus + this.goldenScore - this.livesPenalty;

    this.phaseStartTime = globalState.frameTime;
  }

  private get hugoForwardTime(): number {
    return ScoreboardResources.hugoSide.length / ScoreboardGame.HUGO_ANIM_FPS;
  }

  processEvents(_phoneEvents: PhoneEvents): void {
    const phaseTime   = globalState.frameTime - this.phaseStartTime;
    const forwardTime = this.hugoForwardTime;

    if (this.currentRow < 5) {
      if (this.rowPhase === 0) {
        if (phaseTime >= forwardTime) {
          this.rowPhase = 1;
          this.phaseStartTime = globalState.frameTime;
        }
      } else if (this.rowPhase === 1) {
        this.updateRolling();
        if (this.isRowComplete()) {
          this.rowPhase = 2;
          this.phaseStartTime = globalState.frameTime;
        }
      } else if (this.rowPhase === 2) {
        if (phaseTime >= forwardTime) {
          this.currentRow++;
          this.rowPhase = 0;
          this.phaseStartTime = globalState.frameTime;
        }
      }
    }

    const rollingActive = this.rowPhase === 1 && this.currentRow < 5 && this.anyRolling();
    if (rollingActive && !this.isRolling) {
      this.context.scoreboardScoreCounterId = AudioHelper.play(ScoreboardResources.scoreCounter);
      this.scoreCounterStart = globalState.frameTime;
      this.isRolling = true;
    } else if (rollingActive && this.isRolling && globalState.frameTime - this.scoreCounterStart >= 0.1) {
      this.context.scoreboardScoreCounterId = AudioHelper.play(ScoreboardResources.scoreCounter);
      this.scoreCounterStart = globalState.frameTime;
    } else if (!rollingActive && this.isRolling) {
      AudioHelper.stop(this.context.scoreboardScoreCounterId);
      this.isRolling = false;
    }

    if (this.currentRow >= 5 && globalState.frameTime - this.phaseStartTime > 2.0) {
      this.context.forestScore = this.totalScore;
      this.end();
    }
  }

  private isRowComplete(): boolean {
    if (this.currentRow === 0) return this.rollingSack >= this.sackScore;
    if (this.currentRow === 1) return this.rollingEnd >= this.endBonus;
    if (this.currentRow === 2) return this.rollingGolden >= this.goldenScore;
    if (this.currentRow === 3) return this.rollingPenalty >= this.livesPenalty;
    if (this.currentRow === 4) return this.rollingTotal >= this.totalScore;
    return true;
  }

  private anyRolling(): boolean {
    if (this.currentRow === 0 && this.rollingSack < this.sackScore)       return true;
    if (this.currentRow === 1 && this.rollingEnd < this.endBonus)         return true;
    if (this.currentRow === 2 && this.rollingGolden < this.goldenScore)   return true;
    if (this.currentRow === 3 && this.rollingPenalty < this.livesPenalty) return true;
    if (this.currentRow === 4 && this.rollingTotal < this.totalScore)     return true;
    return false;
  }

  private updateRolling(): void {
    if (this.currentRow >= 0) this.rollingSack    = Math.min(this.sackScore,    this.rollingSack    + ScoreboardGame.ROLL_SPEED);
    if (this.currentRow >= 1) this.rollingEnd     = Math.min(this.endBonus,     this.rollingEnd     + ScoreboardGame.ROLL_SPEED);
    if (this.currentRow >= 2) this.rollingGolden  = Math.min(this.goldenScore,  this.rollingGolden  + ScoreboardGame.ROLL_SPEED);
    if (this.currentRow >= 3) this.rollingPenalty = Math.min(this.livesPenalty, this.rollingPenalty + ScoreboardGame.ROLL_SPEED);
    if (this.currentRow >= 4) this.rollingTotal   = Math.min(this.totalScore,   this.rollingTotal   + ScoreboardGame.ROLL_SPEED);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(ScoreboardResources.background!, 0, 0);

    this.renderHugo(ctx);

    if (this.currentRow >= 0) this.renderRow(ctx, 0, ScoreboardResources.iconSack!,       this.rollingSack,    false);
    if (this.currentRow >= 1) this.renderRow(ctx, 1, ScoreboardResources.iconFinish!,     this.rollingEnd,     false);
    if (this.currentRow >= 2) this.renderRow(ctx, 2, ScoreboardResources.iconGoldenSack!, this.rollingGolden,  false);
    if (this.currentRow >= 3) this.renderLivesLostRow(ctx, 3);
    this.renderTotalRow(ctx, 4);
  }

  private renderHugo(ctx: OffscreenCanvasRenderingContext2D): void {
    if (this.currentRow >= 5) return;

    const progress = Math.min(1.0, this.currentRow / 4.0);
    const hugoY    = ScoreboardGame.HUGO_START_Y + (ScoreboardGame.HUGO_END_Y - ScoreboardGame.HUGO_START_Y) * progress;

    const phaseTime = globalState.frameTime - this.phaseStartTime;
    const numFrames = ScoreboardResources.hugoSide.length;
    let frameIdx: number;

    if (this.rowPhase === 0) {
      frameIdx = Math.min(Math.floor(phaseTime * ScoreboardGame.HUGO_ANIM_FPS), numFrames - 1);
    } else if (this.rowPhase === 1) {
      frameIdx = numFrames - 1;
    } else {
      frameIdx = Math.max(numFrames - 1 - Math.floor(phaseTime * ScoreboardGame.HUGO_ANIM_FPS), 0);
    }

    ctx.drawImage(ScoreboardResources.hugoSide[frameIdx], ScoreboardGame.HUGO_X, Math.floor(hugoY));
  }

  private renderRow(
    ctx: OffscreenCanvasRenderingContext2D,
    rowIndex: number,
    icon: ImageBitmap,
    score: number,
    isNegative: boolean,
  ): void {
    const y = ScoreboardGame.FIRST_ROW_Y + rowIndex * ScoreboardGame.ROW_HEIGHT;
    ctx.drawImage(icon, ScoreboardGame.ICON_X, y);
    this.renderScore(ctx, ScoreboardGame.SCORE_X, y + 8, score, isNegative);
  }

  private renderLivesLostRow(ctx: OffscreenCanvasRenderingContext2D, rowIndex: number): void {
    const y = ScoreboardGame.FIRST_ROW_Y + rowIndex * ScoreboardGame.ROW_HEIGHT;

    const obstaclesHit = this.context.forestObstaclesHit ?? [];
    const iconX = ScoreboardGame.ICON_X;
    const obstacleIcons: Record<number, ImageBitmap | null> = {
      1: ScoreboardResources.iconObstacleCatapult,
      2: ScoreboardResources.iconObstacleTrap,
      3: ScoreboardResources.iconObstacleRock,
      4: ScoreboardResources.iconObstacleBranch,
    };

    for (let i = 0; i < Math.min(obstaclesHit.length, 3); i++) {
      const icon = obstacleIcons[Number(obstaclesHit[i])];
      if (icon) {
        const ix = iconX + i * 28;
        ctx.drawImage(ScoreboardResources.iconDeathBg!, ix - 2, y - 1);
        ctx.drawImage(icon, ix, y);
      }
    }

    this.renderScore(ctx, ScoreboardGame.SCORE_X, y + 8, this.rollingPenalty, true);
  }

  private renderTotalRow(ctx: OffscreenCanvasRenderingContext2D, rowIndex: number): void {
    const y = ScoreboardGame.FIRST_ROW_Y + rowIndex * ScoreboardGame.ROW_HEIGHT;
    ctx.drawImage(ScoreboardResources.iconTotal!, ScoreboardGame.ICON_X - 10, y + 15);
    this.renderScore(ctx, ScoreboardGame.SCORE_X, y + 8, this.rollingTotal, false);
  }

  private renderScore(
    ctx: OffscreenCanvasRenderingContext2D,
    x: number,
    y: number,
    score: number,
    isNegative: boolean,
  ): void {
    score = Math.floor(score);

    if (isNegative && score > 0) {
      ctx.drawImage(ScoreboardResources.iconNegative!, x - 17, y);
    }

    // Always render 6 digits (zero-padded on the left)
    const digits: number[] = [];
    let temp = Math.abs(score);
    for (let i = 0; i < 6; i++) {
      digits.unshift(temp % 10);
      temp = Math.floor(temp / 10);
    }

    for (let i = 0; i < digits.length; i++) {
      ctx.drawImage(
        ScoreboardResources.scoreFont[digits[i]],
        x + i * ScoreboardGame.SCORE_DIGIT_SPACING,
        y,
      );
    }
  }

  end(): void {
    if (this.isRolling) {
      AudioHelper.stop(this.context.scoreboardScoreCounterId);
      this.isRolling = false;
    }
    this.ended = true;
  }
}
