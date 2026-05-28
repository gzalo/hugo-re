/** Port of game/scores/scores.py. Uses localStorage instead of SQLite. */

const STORAGE_KEY = 'hugo_re_scores';

interface ScoreEntry {
  gameName: string;
  playerName: string;
  score: number;
}

function load(): ScoreEntry[] {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    return raw ? JSON.parse(raw) : [];
  } catch {
    return [];
  }
}

function save(entries: ScoreEntry[]): void {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(entries));
}

export class Scores {
  private entries: ScoreEntry[];

  constructor() {
    this.entries = load();
  }

  insertScore(gameName: string, playerName: string, score: number): void {
    if (playerName === '') return;
    this.entries.push({ gameName, playerName, score });
    save(this.entries);
  }

  getTopScores(gameName: string, limit = 10): [string, number][] {
    return this.entries
      .filter(e => e.gameName === gameName)
      .sort((a, b) => b.score - a.score)
      .slice(0, limit)
      .map(e => [e.playerName, e.score]);
  }

  initDummyScores(gameOptions: string[]): void {
    for (const gameName of gameOptions) {
      for (let score = 0; score < 5; score++) {
        this.insertScore(gameName, 'Hugo', score);
      }
    }
  }
}
