/** Port of game/forest/forest_resources.py */

import { loadSurfaces, loadSurfaceRaw, loadSurfaceRes, loadSync, loadSpeak, loadSfx } from '../resource';

/** Make all pure-black (0,0,0) pixels fully transparent — mirrors Python colorkey fix on bg_hillsday. */
async function makeBlackTransparent(bitmap: ImageBitmap): Promise<ImageBitmap> {
  const canvas = new OffscreenCanvas(bitmap.width, bitmap.height);
  const ctx = canvas.getContext('2d')!;
  ctx.drawImage(bitmap, 0, 0);
  const img = ctx.getImageData(0, 0, bitmap.width, bitmap.height);
  const d = img.data;
  for (let i = 0; i < d.length; i += 4) {
    if (d[i] === 0 && d[i + 1] === 0 && d[i + 2] === 0) d[i + 3] = 0;
  }
  ctx.putImageData(img, 0, 0);
  return createImageBitmap(canvas);
}

export class ForestResources {
  static bgHillsday: ImageBitmap[] = [];
  static bgTrees: ImageBitmap[] = [];
  static bgSpookyTrees: ImageBitmap[] = [];
  static bgGround: ImageBitmap[] = [];
  static grass: ImageBitmap[] = [];
  static leaves1: ImageBitmap[] = [];
  static leaves2: ImageBitmap[] = [];
  static scoreboard: ImageBitmap | null = null;
  static bgGradient: ImageBitmap | null = null;
  static endMountain: ImageBitmap[] = [];
  static hugoSide: ImageBitmap[] = [];
  static hugoCrawl: ImageBitmap[] = [];
  static hugoJump: ImageBitmap[] = [];
  static hugoTelllives: ImageBitmap[] = [];
  static hugoHand1: ImageBitmap[] = [];
  static hugoHand2: ImageBitmap[] = [];
  static tree: ImageBitmap[] = [];
  static loneTree: ImageBitmap[] = [];
  static rock: ImageBitmap[] = [];
  static sack: ImageBitmap[] = [];
  static trap: ImageBitmap[] = [];
  static catapult: ImageBitmap[] = [];
  static hitRock: ImageBitmap[] = [];
  static hugoLookrock: ImageBitmap[] = [];
  static hitRockSync: ImageBitmap[] = [];
  static catapultFly: ImageBitmap[] = [];
  static catapultFall: ImageBitmap[] = [];
  static catapultAirtalk: ImageBitmap[] = [];
  static catapultHang: ImageBitmap[] = [];
  static catapultHangspeak: ImageBitmap[] = [];
  static hugohitlog: ImageBitmap[] = [];
  static hugohitlogTalk: ImageBitmap[] = [];
  static hugoTraptalk: ImageBitmap[] = [];
  static hugoTraphurt: ImageBitmap[] = [];
  static scoreNumbers: ImageBitmap[] = [];
  static hugoLives: ImageBitmap[] = [];
  static scullaHand1: ImageBitmap[] = [];
  static scullaHand2: ImageBitmap[] = [];
  static arrows: ImageBitmap[] = [];

  static syncStart: number[] = [];
  static syncRock: number[] = [];
  static syncDieonce: number[] = [];
  static syncTrap: number[] = [];
  static syncLastlife: number[] = [];
  static syncCatapultTalktop: number[] = [];
  static syncCatapultHang: number[] = [];
  static syncHitlog: number[] = [];
  static syncGameover: number[] = [];
  static syncLevelcompleted: number[] = [];

  static speakStart = '';
  static speakRock = '';
  static speakDieonce = '';
  static speakTrap = '';
  static speakLastlife = '';
  static speakCatapultUp = '';
  static speakCatapultHit = '';
  static speakCatapultTalktop = '';
  static speakCatapultDown = '';
  static speakCatapultHang = '';
  static speakHitlog = '';
  static speakGameover = '';
  static speakLevelcompleted = '';

  static sfxBgAtmosphere = '';
  static sfxLightningWarning = '';
  static sfxHugoKnock = '';
  static sfxHugoHittrap = '';
  static sfxHugoLaunch = '';
  static sfxSackNormal = '';
  static sfxSackBonus = '';
  static sfxTreeSwush = '';
  static sfxHugoHitlog = '';
  static sfxCatapultEject = '';
  static sfxBirds = '';
  static sfxHugoHitscreen = '';
  static sfxHugoScreenklir = '';
  static sfxHugoCrash = '';
  static sfxHugoHangstart = '';
  static sfxHugoHang = '';
  static sfxHugoWalk0 = '';
  static sfxHugoWalk1 = '';
  static sfxHugoWalk2 = '';
  static sfxHugoWalk3 = '';
  static sfxHugoWalk4 = '';
  static sfxHintDos = '';
  static sfxHintOcho = '';

  static invertedArrows: ImageBitmap | null = null;
  static hintOverlayDos: ImageBitmap | null = null;
  static hintOverlayOcho: ImageBitmap | null = null;

  static async init(): Promise<void> {

    [
      ForestResources.bgHillsday,
      ForestResources.bgTrees,
      ForestResources.bgSpookyTrees,
      ForestResources.bgGround,
      ForestResources.grass,
      ForestResources.leaves1,
      ForestResources.leaves2,
      ForestResources.endMountain,
      ForestResources.hugoSide,
      ForestResources.hugoCrawl,
      ForestResources.hugoJump,
      ForestResources.hugoTelllives,
      ForestResources.hugoHand1,
      ForestResources.hugoHand2,
      ForestResources.tree,
      ForestResources.loneTree,
      ForestResources.rock,
      ForestResources.sack,
      ForestResources.trap,
      ForestResources.catapult,
      ForestResources.hitRock,
      ForestResources.hugoLookrock,
      ForestResources.hitRockSync,
      ForestResources.catapultFly,
      ForestResources.catapultFall,
      ForestResources.catapultAirtalk,
      ForestResources.catapultHang,
      ForestResources.catapultHangspeak,
      ForestResources.hugohitlog,
      ForestResources.hugohitlogTalk,
      ForestResources.hugoTraptalk,
      ForestResources.hugoTraphurt,
      ForestResources.scoreNumbers,
      ForestResources.hugoLives,
      ForestResources.scullaHand1,
      ForestResources.scullaHand2,
    ] = await Promise.all([
      loadSurfaces('ForestData', 'hillsday.cgf',              0,   0),
      loadSurfaces('ForestData', 'paratrees.cgf',             0,   0),
      loadSurfaces('ForestData', 'spooky.cgf',                0,   0),
      loadSurfaces('ForestData', 'paraground.cgf',            0,   0),
      loadSurfaces('ForestData', 'GRASS.cgf',                 0,   0),
      loadSurfaces('ForestData', 'LEAVES1.cgf',               0,   0),
      loadSurfaces('ForestData', 'LEAVES2.cgf',               0,   0),
      loadSurfaces('ForestData', 'WALL.cgf',                  0,   0),
      loadSurfaces('ForestData', 'hugoside.cgf',              0,   7),
      loadSurfaces('ForestData', 'kravle.cgf',                0,   7),
      loadSurfaces('ForestData', 'hugohop.cgf',               0,   2),
      loadSurfaces('ForestData', 'hugo_hello.cgf',            0,  15),
      loadSurfaces('ForestData', 'hand1.cgf',                 0,   0),
      loadSurfaces('ForestData', 'hand2.cgf',                 0,   0),
      loadSurfaces('ForestData', 'branch-swing.cgf',          0,   6),
      loadSurfaces('ForestData', 'lonetree.cgf',              0,   0),
      loadSurfaces('ForestData', 'stone.cgf',                 0,   7),
      loadSurfaces('ForestData', 'saek.cgf',                  0,   3),
      loadSurfaces('ForestData', 'faelde.cgf',                0,   5),
      loadSurfaces('ForestData', 'catapult.cgf',              0,   7),
      loadSurfaces('ForestData', 'HGROCK.TIL',                0,  60),
      loadSurfaces('ForestData', 'hugo-rock.til',             0,  14),
      loadSurfaces('ForestData', 'MSYNCRCK.TIL',              0,  17),
      loadSurfaces('ForestData', 'HGKATFLY.til',              0, 113),
      loadSurfaces('ForestData', 'HGKATFLY.til',            115, 189),
      loadSurfaces('ForestData', 'CATAPULT-SPEAK.til',        0,  15),
      loadSurfaces('ForestData', 'HGKATHNG.TIL',              0,  12),
      loadSurfaces('ForestData', 'hanging_mouth.cgf',         0,  11),
      loadSurfaces('ForestData', 'BRANCH-GROGGY.til',         0,  42),
      loadSurfaces('ForestData', 'BRANCH-SPEAK.til',          0,  17),
      loadSurfaces('ForestData', 'traptalk.til',              0,  15),
      loadSurfaces('ForestData', 'TRAP-HURTS.til',            0,   9),
      loadSurfaces('ForestData', 'SCORES.cgf',                0,   0),
      loadSurfaces('ForestData', 'HUGOSTAT.cgf',              0,   0),
      loadSurfaces('ForestData', 'HEKS1.cgf',                 0,   0),
      loadSurfaces('ForestData', 'HEKS2.cgf',                 0,   0),
    ]);

    // Mirrors Python: make pure-black pixels transparent in the hills layer
    ForestResources.bgHillsday = await Promise.all(
      ForestResources.bgHillsday.map(makeBlackTransparent),
    );

    [
      ForestResources.scoreboard,
      ForestResources.bgGradient,
      ForestResources.invertedArrows,
      ForestResources.hintOverlayDos,
      ForestResources.hintOverlayOcho,
    ] = await Promise.all([
      loadSurfaceRaw('ForestData', 'SCOREBRD.bmp'),
      loadSurfaceRes('fixed_assets/gradient.bmp'),
      loadSurfaceRes('fixed_assets/inverted_arrows.png'),
      loadSurfaceRes('fixed_assets/hint_dos.png'),
      loadSurfaceRes('fixed_assets/hint_ocho.png'),
    ]);

    ForestResources.arrows = await Promise.all(
      [0, 1, 2, 3].map(i => loadSurfaceRes(`fixed_assets/arrows.cgf_${i}.png`))
    );

    [
      ForestResources.syncStart,
      ForestResources.syncRock,
      ForestResources.syncDieonce,
      ForestResources.syncTrap,
      ForestResources.syncLastlife,
      ForestResources.syncCatapultTalktop,
      ForestResources.syncCatapultHang,
      ForestResources.syncHitlog,
      ForestResources.syncGameover,
      ForestResources.syncLevelcompleted,
    ] = await Promise.all([
      loadSync('ForestData', '005-01.oos'),
      loadSync('ForestData', '005-02.oos'),
      loadSync('ForestData', '005-03.oos'),
      loadSync('ForestData', '005-04.oos'),
      loadSync('ForestData', '005-05.oos'),
      loadSync('ForestData', '005-08.oos'),
      loadSync('ForestData', '005-10.oos'),
      loadSync('ForestData', '005-11.oos'),
      loadSync('ForestData', '005-12.oos'),
      loadSync('ForestData', '005-13.oos'),
    ]);

    // Audio paths (strings only — no preloading here, audioHelper handles that on demand)
    ForestResources.speakStart             = loadSpeak('ForestData', '005-01.wav');
    ForestResources.speakRock              = loadSpeak('ForestData', '005-02.wav');
    ForestResources.speakDieonce           = loadSpeak('ForestData', '005-03.wav');
    ForestResources.speakTrap              = loadSpeak('ForestData', '005-04.wav');
    ForestResources.speakLastlife          = loadSpeak('ForestData', '005-05.wav');
    ForestResources.speakCatapultUp        = loadSpeak('ForestData', '005-06.wav');
    ForestResources.speakCatapultHit       = loadSpeak('ForestData', '005-07.wav');
    ForestResources.speakCatapultTalktop   = loadSpeak('ForestData', '005-08.wav');
    ForestResources.speakCatapultDown      = loadSpeak('ForestData', '005-09.wav');
    ForestResources.speakCatapultHang      = loadSpeak('ForestData', '005-10.wav');
    ForestResources.speakHitlog            = loadSpeak('ForestData', '005-11.wav');
    ForestResources.speakGameover          = loadSpeak('ForestData', '005-12.wav');
    ForestResources.speakLevelcompleted    = loadSpeak('ForestData', '005-13.wav');

    ForestResources.sfxBgAtmosphere        = loadSfx('ForestData', 'atmos-lp.wav');
    ForestResources.sfxLightningWarning    = loadSfx('ForestData', 'warning.wav');
    ForestResources.sfxHugoKnock           = loadSfx('ForestData', 'knock.wav');
    ForestResources.sfxHugoHittrap         = loadSfx('ForestData', 'crunch.wav');
    ForestResources.sfxHugoLaunch          = loadSfx('ForestData', 'skriid.wav');
    ForestResources.sfxSackNormal          = loadSfx('ForestData', 'sack-norm.wav');
    ForestResources.sfxSackBonus           = loadSfx('ForestData', 'sack.wav');
    ForestResources.sfxTreeSwush           = loadSfx('ForestData', 'wush.wav');
    ForestResources.sfxHugoHitlog          = loadSfx('ForestData', 'bell.wav');
    ForestResources.sfxCatapultEject       = loadSfx('ForestData', 'fjeder.wav');
    ForestResources.sfxBirds               = loadSfx('ForestData', 'birds-lp.wav');
    ForestResources.sfxHugoHitscreen       = loadSfx('ForestData', 'hit_screen.wav');
    ForestResources.sfxHugoScreenklir      = loadSfx('ForestData', 'klirr.wav');
    ForestResources.sfxHugoCrash           = loadSfx('ForestData', 'kineser.wav');
    ForestResources.sfxHugoHangstart       = loadSfx('ForestData', 'knage-start.wav');
    ForestResources.sfxHugoHang            = loadSfx('ForestData', 'knage-lp.wav');
    ForestResources.sfxHugoWalk0           = loadSfx('ForestData', 'fumle0.wav');
    ForestResources.sfxHugoWalk1           = loadSfx('ForestData', 'fumle1.wav');
    ForestResources.sfxHugoWalk2           = loadSfx('ForestData', 'fumle2.wav');
    ForestResources.sfxHugoWalk3           = loadSfx('ForestData', 'fumle3.wav');
    ForestResources.sfxHugoWalk4           = loadSfx('ForestData', 'fumle4.wav');
    ForestResources.sfxHintDos             = 'sfx/dos.wav';
    ForestResources.sfxHintOcho            = 'sfx/ocho.wav';
  }
}
