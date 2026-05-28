/** Port of game/scoreboard/scoreboard_resources.py */

import { loadSurfaceRes, loadSurfaces, loadSfx } from '../resource';

async function cropBitmap(
  src: ImageBitmap, sx: number, sy: number, sw: number, sh: number,
  label: string,
): Promise<ImageBitmap> {
  try {
    return await createImageBitmap(src, sx, sy, sw, sh);
  } catch (e) {
    throw new Error(
      `ScoreboardResources: createImageBitmap crop failed for "${label}" ` +
      `at (${sx},${sy},${sw}×${sh}) on ${src.width}×${src.height} image — ${e}`,
    );
  }
}

export class ScoreboardResources {
  static background:             ImageBitmap | null = null;
  static hugoSide:               ImageBitmap[]      = [];

  static iconSack:               ImageBitmap | null = null;
  static iconFinish:             ImageBitmap | null = null;
  static iconGoldenSack:         ImageBitmap | null = null;
  static iconTotal:              ImageBitmap | null = null;

  static iconObstacleCatapult:   ImageBitmap | null = null;
  static iconObstacleTrap:       ImageBitmap | null = null;
  static iconObstacleRock:       ImageBitmap | null = null;
  static iconObstacleBranch:     ImageBitmap | null = null;
  static iconDeathBg:            ImageBitmap | null = null;

  static iconNegative:           ImageBitmap | null = null;

  static scoreFont:              ImageBitmap[]      = [];
  static scoreCounter            = '';

  static async init(): Promise<void> {
    const [sprite1, sprite2] = await Promise.all([
      loadSurfaceRes('scores/sprite1.png'),
      loadSurfaceRes('scores/sprite2.png'),
    ]);

    // Background: crop (464,144,320,256) then scale to (300,240)
    const bgCrop = await cropBitmap(sprite1, 464, 144, 320, 256, 'background');
    const bgCanvas = new OffscreenCanvas(300, 240);
    bgCanvas.getContext('2d')!.drawImage(bgCrop, 0, 0, 300, 240);
    ScoreboardResources.background = await createImageBitmap(bgCanvas);

    // Hugo side-walk animation: 4 rows × 4 cols
    const hugoSidePromises: Promise<ImageBitmap>[] = [];
    for (const rowY of [40, 134, 228, 322]) {
      for (const colX of [17, 129, 241, 353]) {
        hugoSidePromises.push(cropBitmap(sprite1, colX, rowY, 93, 77, `hugoSide@(${colX},${rowY})`));
      }
    }
    ScoreboardResources.hugoSide = await Promise.all(hugoSidePromises);

    // Row icons from sprite1
    const [
      iconSack,
      iconGoldenSack,
      iconFinish,
      iconTotal,
      iconDeathBg,
      iconNegative,
    ] = await Promise.all([
      cropBitmap(sprite1, 465,  35, 62, 52, 'iconSack'),
      cropBitmap(sprite1, 538,  39, 42, 48, 'iconGoldenSack'),
      cropBitmap(sprite1, 471,  91, 52, 50, 'iconFinish'),
      cropBitmap(sprite1, 592, 108, 73, 15, 'iconTotal'),
      cropBitmap(sprite1, 592,  41, 50, 46, 'iconDeathBg'),
      cropBitmap(sprite1, 675,  92, 15, 24, 'iconNegative'),
    ]);
    ScoreboardResources.iconSack        = iconSack;
    ScoreboardResources.iconGoldenSack  = iconGoldenSack;
    ScoreboardResources.iconFinish      = iconFinish;
    ScoreboardResources.iconTotal       = iconTotal;
    ScoreboardResources.iconDeathBg     = iconDeathBg;
    ScoreboardResources.iconNegative    = iconNegative;

    // Obstacle icons from sprite2
    const [
      iconObstacleCatapult,
      iconObstacleTrap,
      iconObstacleRock,
      iconObstacleBranch,
    ] = await Promise.all([
      cropBitmap(sprite2, 165, 149, 38, 38, 'iconObstacleCatapult'),
      cropBitmap(sprite2, 117, 149, 38, 38, 'iconObstacleTrap'),
      cropBitmap(sprite2,  69, 149, 38, 38, 'iconObstacleRock'),
      cropBitmap(sprite2,  69, 149, 38, 38, 'iconObstacleBranch'),
    ]);
    ScoreboardResources.iconObstacleCatapult = iconObstacleCatapult;
    ScoreboardResources.iconObstacleTrap     = iconObstacleTrap;
    ScoreboardResources.iconObstacleRock     = iconObstacleRock;
    ScoreboardResources.iconObstacleBranch   = iconObstacleBranch;

    [ScoreboardResources.scoreFont] = await Promise.all([
      loadSurfaces('RopeOutroData', 'SCORE.cgf', 0, 9),
    ]);

    ScoreboardResources.scoreCounter = loadSfx('RopeOutroData', 'COUNTER.WAV');
  }
}
