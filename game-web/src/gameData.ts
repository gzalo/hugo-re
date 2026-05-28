export class GameData {
  country: string;
  audioPort: number;  // kept for reference; not used (Web Audio replaces UDP)

  // Forest game state
  forestScore: number;
  forestLives: number;
  forestParallaxPos: number;
  forestSacks: unknown[];
  forestObstacles: unknown[];
  forestLeaves: unknown[];
  forestControlsInverted: boolean;

  // Forest scoring
  forestNormalSacksCollected = 0;
  forestGoldenSacksCollected = 0;
  forestReachedEnd = false;
  forestObstaclesHit: string[] = [];

  // Cave game state
  caveSelectedRope = 0;
  caveWinType = 0;

  // Audio instance IDs (Web Audio AudioBufferSourceNode wrappers)
  forestBgAtmosphereId: number | null = null;
  caveScoreCounterId: number | null = null;
  caveFanfareId: number | null = null;
  caveBgMusicId: number | null = null;
  caveStemningId: number | null = null;
  scoreboardScoreCounterId: number | null = null;

  constructor(
    country: string,
    audioPort: number,
    forestScore: number,
    forestLives: number,
    forestParallaxPos: number,
    forestSacks: unknown[],
    forestObstacles: unknown[],
    forestLeaves: unknown[],
    forestControlsInverted: boolean,
    forestNormalSacksCollected: number,
    forestGoldenSacksCollected: number,
  ) {
    this.country = country;
    this.audioPort = audioPort;
    this.forestScore = forestScore;
    this.forestLives = forestLives;
    this.forestParallaxPos = forestParallaxPos;
    this.forestSacks = forestSacks;
    this.forestObstacles = forestObstacles;
    this.forestLeaves = forestLeaves;
    this.forestControlsInverted = forestControlsInverted;
    this.forestNormalSacksCollected = forestNormalSacksCollected;
    this.forestGoldenSacksCollected = forestGoldenSacksCollected;
  }
}
