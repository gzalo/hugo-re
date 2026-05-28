/** Port of game/scores/high_score_renderer.py */

import { globalState } from '../globalState';
import type { Scores } from './scores';

export class HighScoreRenderer {
  private scores: Scores;
  private scoreGame: string;
  private games: Record<string, { name: string }>;
  private timeScore: number;

  constructor(scores: Scores, games: Record<string, { name: string }>) {
    this.scores = scores;
    this.games = games;
    this.scoreGame = Object.keys(games)[0];
    this.timeScore = globalState.frameTime;
  }

  setRandomScoreGame(): void {
    const keys = Object.keys(this.games);
    if (keys.length === 1) { this.scoreGame = keys[0]; return; }
    let newGame = keys[Math.floor(Math.random() * keys.length)];
    while (newGame === this.scoreGame) {
      newGame = keys[Math.floor(Math.random() * keys.length)];
    }
    this.scoreGame = newGame;
  }

  renderOutline(ctx: OffscreenCanvasRenderingContext2D, text: string, xpos: number, ypos: number): void {
    ctx.font = 'bold 28px Arial';
    const offsets = [-1, 1, -2, 2];
    ctx.fillStyle = '#000';
    for (const dx of offsets) {
      for (const dy of offsets) {
        ctx.fillText(text, xpos + dx, ypos + dy);
      }
    }
    ctx.fillStyle = '#fff';
    ctx.fillText(text, xpos, ypos);
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    if (globalState.frameTime - this.timeScore > 4) {
      this.setRandomScoreGame();
      this.timeScore = globalState.frameTime;
    }

    this.renderOutline(ctx, 'PUNTAJES ' + this.games[this.scoreGame].name, 30, 30);

    const topScores = this.scores.getTopScores(this.scoreGame, 5);
    topScores.forEach(([name, score], i) => {
      this.renderOutline(ctx, `${i + 1}. ${name} - ${score}`, 30, 30 + (i + 1) * 30);
    });
  }
}
