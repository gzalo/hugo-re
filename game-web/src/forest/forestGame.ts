/** Port of game/forest/forest_game.py */

import { globalState } from '../globalState';
import { AudioHelper } from '../audioHelper';
import { Config } from '../config';
import { EffectType } from '../effectType';
import { ForestResources } from './forestResources';
import { WaitIntro } from './waitIntro';
import { Playing } from './playing';
import { NullState } from '../nullState';
import { Splat } from '../effects/splat';
import { RenderType } from '../renderType';
import { State, type StateClass } from '../state';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class ForestGame {
  context: GameData;
  ended = false;

  private _state: State;
  effectStatus: EffectType | null = null;
  effectStart: number | null = null;
  private forestBgAtmosphereStart: number;

  constructor(context: GameData) {
    this.context = context;
    context.forestScore = 0;
    context.forestLives = 3;
    context.forestParallaxPos = 0;
    context.forestObstacles = ForestGame.generateObstacles();
    context.forestSacks = ForestGame.generateSacks();
    context.forestLeaves = ForestGame.generateLeaves();

    this._state = new WaitIntro(context);
    this._state.onEnter();

    context.forestBgAtmosphereId = AudioHelper.play(ForestResources.sfxBgAtmosphere);
    this.forestBgAtmosphereStart = globalState.frameTime;
  }

  processEvents(phoneEvents: PhoneEvents): void {
    const invertActive =
      this.effectStatus === EffectType.INVERT &&
      globalState.frameTime - this.effectStart! > Config.EFFECT_DURATION_ORB;

    if (invertActive) {
      [phoneEvents.press_2, phoneEvents.press_8] = [phoneEvents.press_8, phoneEvents.press_2];
    }
    this.context.forestControlsInverted = invertActive;

    const nextStateClass = this._state.processEvents(phoneEvents);

    if (this.effectStatus === EffectType.SPLAT && globalState.frameTime - this.effectStart! > Config.EFFECT_DURATION_SPLAT) {
      this.effectStatus = null;
    }
    if (this.effectStatus === EffectType.INVERT && globalState.frameTime - this.effectStart! > Config.EFFECT_DURATION_INVERT) {
      this.effectStatus = null;
      this.context.forestControlsInverted = false;
    }

    if (nextStateClass !== null) {
      this._state.onExit();
      if (nextStateClass === NullState) {
        this.end();
      } else {
        this._state = new (nextStateClass as StateClass)(this.context);
        this._state.onEnter();
      }
    }

    if (!this.ended && globalState.frameTime - this.forestBgAtmosphereStart >= 5.5) {
      this.context.forestBgAtmosphereId = AudioHelper.play(ForestResources.sfxBgAtmosphere);
      this.forestBgAtmosphereStart = globalState.frameTime;
    }
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    const needsBg   = (this._state as any).needsBackground;
    const needsBot  = (this._state as any).needsBottom;

    if (needsBg  === RenderType.PRE) this.renderBackground(ctx);
    if (needsBot === RenderType.PRE) this.renderBottom(ctx);

    this._state.render(ctx);

    if (needsBg  === RenderType.POST) this.renderBackground(ctx);
    if (needsBot === RenderType.POST) this.renderBottom(ctx);

    if (this.effectStart !== null) {
      const dt = globalState.frameTime - this.effectStart;
      if (dt < Config.EFFECT_DURATION_ORB) return;

      if (this.effectStatus === EffectType.SPLAT) {
        Splat.renderSplat(ctx, globalState.frameTime - this.effectStart);
      }
      if (this.effectStatus === EffectType.INVERT && this._state instanceof Playing) {
        ctx.drawImage(ForestResources.invertedArrows!, 0, 0);
      }
    }
  }

  externalEffect(effect: EffectType): void {
    if (this.effectStatus !== null) return;
    this.effectStatus = effect;
    this.effectStart = globalState.frameTime;
  }

  private renderBackground(ctx: OffscreenCanvasRenderingContext2D): void {
    const hillsSpeed  = 6  * Config.FOREST_BG_SPEED_MULTIPLIER;
    const treesSpeed  = 12 * Config.FOREST_BG_SPEED_MULTIPLIER;
    const grassSpeed  = 30 * Config.FOREST_BG_SPEED_MULTIPLIER;

    const hillsWidth  = ForestResources.bgHillsday[0].width;
    const treesWidth  = ForestResources.bgTrees[0].width;
    const grassWidth  = ForestResources.grass[0].width;
    const groundWidth = ForestResources.bgGround[0].width;

    const hillsX  = ForestGame.newMod(-this.context.forestParallaxPos * hillsSpeed,  hillsWidth);
    const treesX  = ForestGame.newMod(-this.context.forestParallaxPos * treesSpeed,  treesWidth);
    const grassX  = ForestGame.newMod(-this.context.forestParallaxPos * grassSpeed,  grassWidth);
    const groundX = ForestGame.newMod(-this.context.forestParallaxPos * Config.FOREST_GROUND_SPEED, groundWidth);

    const mountainPos = 320 - 96 - (this.context.forestParallaxPos - Config.FOREST_MAX_TIME) * Config.FOREST_GROUND_SPEED;

    ctx.drawImage(ForestResources.bgGradient!, 0, 0);
    ctx.drawImage(ForestResources.bgHillsday[0], hillsX, 0);
    ctx.drawImage(ForestResources.bgHillsday[0], hillsX + hillsWidth, 0);
    ctx.drawImage(ForestResources.bgTrees[0], treesX, -24);
    ctx.drawImage(ForestResources.bgTrees[0], treesX + treesWidth, -24);

    for (let i = 0; i < 5; i++) ctx.drawImage(ForestResources.bgGround[0], groundX + i * groundWidth, 158);
    for (let i = 0; i < 12; i++) ctx.drawImage(ForestResources.grass[0], grassX + i * grassWidth, 172);

    ctx.drawImage(ForestResources.endMountain[0], mountainPos, -16);
  }

  private renderBottom(ctx: OffscreenCanvasRenderingContext2D): void {
    ctx.drawImage(ForestResources.scoreboard!, 0, 184);
    for (let i = 0; i < this.context.forestLives; i++) {
      ctx.drawImage(ForestResources.hugoLives[0], i * 40 + 32, 188);
    }

    const xScoreRight = 150 + 24 * 6;
    const yScore = 194;
    const xSpace = 24;
    const score = Math.floor(this.context.forestScore);

    if (score === 0) {
      const [sx, sy, sw, sh] = ForestGame.getSpritesheetArea(0);
      ctx.drawImage(ForestResources.scoreNumbers[0], sx, sy, sw, sh, xScoreRight - xSpace, yScore, sw, sh);
      return;
    }

    const digits: number[] = [];
    let temp = score;
    while (temp > 0) { digits.push(temp % 10); temp = Math.floor(temp / 10); }

    for (let i = 0; i < digits.length; i++) {
      const [sx, sy, sw, sh] = ForestGame.getSpritesheetArea(digits[i]);
      ctx.drawImage(ForestResources.scoreNumbers[0], sx, sy, sw, sh, xScoreRight - xSpace * (i + 1), yScore, sw, sh);
    }
  }

  end(): void {
    this._state.onExit();
    AudioHelper.stop(this.context.forestBgAtmosphereId);
    this.ended = true;
  }

  static getSpritesheetArea(value: number): [number, number, number, number] {
    const width = 32, height = 33;
    const xpos = value % 5;
    const ypos = Math.floor(value / 5);
    return [1 + xpos * (width + 1), 1 + ypos * (height + 1), width, height];
  }

  static newMod(a: number, b: number): number {
    const res = a % b;
    if (!res) return res;
    return a < 0 ? res - b : res;
  }

  static generateObstacles(): number[] {
    const emptyProb = 0.65;
    const otherProb = (1 - emptyProb) / 4;
    const weights = [emptyProb, otherProb, otherProb, otherProb, otherProb];
    const out = weightedChoices([0, 1, 2, 3, 4], weights, Config.FOREST_MAX_TIME);
    out[0] = 0; out[1] = 0; out[2] = 0; out[3] = 0;
    for (let i = 0; i < out.length - 1; i++) if (out[i] !== 0) out[i + 1] = 0;
    return out;
  }

  static generateSacks(): number[] {
    const emptyProb = 0.7;
    const otherProb = (1 - emptyProb) * 0.7;
    const otherProbHigh = (1 - emptyProb) * 0.3;
    return weightedChoices([0, 1, 2], [emptyProb, otherProb, otherProbHigh], Config.FOREST_MAX_TIME);
  }

  static generateLeaves(): number[] {
    const arr = weightedChoices([1, 2], [0.5, 0.5], Config.FOREST_MAX_TIME);
    for (let i = 0; i < arr.length - 1; i++) if (arr[i] === 2) arr[i + 1] = 0;
    return arr;
  }
}

function weightedChoices(choices: number[], weights: number[], k: number): number[] {
  const total = weights.reduce((a, b) => a + b, 0);
  const out: number[] = [];
  for (let i = 0; i < k; i++) {
    let r = Math.random() * total;
    for (let j = 0; j < choices.length; j++) {
      r -= weights[j];
      if (r <= 0) { out.push(choices[j]); break; }
    }
    if (out.length < i + 1) out.push(choices[choices.length - 1]);
  }
  return out;
}
