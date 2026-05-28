/** Port of game/cave/cave_resources.py */

import { loadSurfaces, loadSurfaceRes, loadSync, loadSpeak, loadSfx } from '../resource';

export class CaveResources {
  static readonly CASELIVE_TIL = 'CASELIVE.TIL';
  static readonly CASEDIE_TIL  = 'CASEDIE.TIL';

  static herErVi     = '';
  static talks:       ImageBitmap[] = [];
  static climbs:      ImageBitmap[] = [];
  static firstRope:   ImageBitmap[] = [];
  static secondRope:  ImageBitmap[] = [];
  static thirdRope:   ImageBitmap[] = [];
  static scyllaLeaves: ImageBitmap[] = [];
  static scyllaBird:  ImageBitmap[] = [];
  static scyllaRopes: ImageBitmap[] = [];
  static scyllaSpring: ImageBitmap[] = [];
  static familyCage:  ImageBitmap[] = [];
  static hugoPuffFirst:  ImageBitmap[] = [];
  static hugoPuffSecond: ImageBitmap[] = [];
  static hugoPuffThird:  ImageBitmap[] = [];
  static hugoSpring:  ImageBitmap[] = [];
  static happy:       ImageBitmap[] = [];
  static scoreFont:   ImageBitmap[] = [];
  static hugo:        ImageBitmap[] = [];

  static syncHugoStart: number[] = [];
  static syncHugoDie:   number[] = [];

  static trappeSnak       = '';
  static nuKommerJeg      = '';
  static afskyliaSnak     = '';
  static hugoKatapult     = '';
  static hugoSkydUd       = '';
  static afskyliaSkyudUd  = '';
  static hugolineTak      = '';

  static stemning     = '';
  static fodtrin1     = '';
  static fodtrin2     = '';
  static hivIReb      = '';
  static fjeder       = '';
  static prePuf       = '';
  static puf          = '';
  static tastTrykket  = '';
  static preFanfare   = '';
  static fanfare      = '';
  static fugleSkrig   = '';
  static trappeGrin   = '';
  static skrig        = '';
  static scoreCounter = '';
  static bgMusic      = 'music/a_party_for_me.mp3';

  static async init(): Promise<void> {
    [
      CaveResources.talks,
      CaveResources.climbs,
      CaveResources.firstRope,
      CaveResources.secondRope,
      CaveResources.thirdRope,
      CaveResources.scyllaLeaves,
      CaveResources.scyllaBird,
      CaveResources.scyllaRopes,
      CaveResources.scyllaSpring,
      CaveResources.familyCage,
      CaveResources.hugoPuffFirst,
      CaveResources.hugoPuffSecond,
      CaveResources.hugoPuffThird,
      CaveResources.hugoSpring,
      CaveResources.happy,
      CaveResources.scoreFont,
      CaveResources.hugo,
    ] = await Promise.all([
      loadSurfaces('RopeOutroData', 'STAIRS.TIL',               0,  12),
      loadSurfaces('RopeOutroData', 'STAIRS.TIL',              11,  51),
      loadSurfaces('RopeOutroData', CaveResources.CASELIVE_TIL, 0,  32),
      loadSurfaces('RopeOutroData', CaveResources.CASELIVE_TIL,33,  72),
      loadSurfaces('RopeOutroData', CaveResources.CASELIVE_TIL,73, 121),
      loadSurfaces('RopeOutroData', CaveResources.CASELIVE_TIL,122, 177),
      loadSurfaces('RopeOutroData', CaveResources.CASELIVE_TIL,178, 240),
      loadSurfaces('RopeOutroData', CaveResources.CASELIVE_TIL,241, 283),
      loadSurfaces('RopeOutroData', CaveResources.CASELIVE_TIL,284, 318),
      loadSurfaces('RopeOutroData', CaveResources.CASELIVE_TIL,319, 352),
      loadSurfaces('RopeOutroData', CaveResources.CASEDIE_TIL, 122, 166),
      loadSurfaces('RopeOutroData', CaveResources.CASEDIE_TIL, 167, 211),
      loadSurfaces('RopeOutroData', CaveResources.CASEDIE_TIL, 212, 256),
      loadSurfaces('RopeOutroData', CaveResources.CASEDIE_TIL, 257, 295),
      loadSurfaces('RopeOutroData', 'HAPPY.TIL',                0, 111),
      loadSurfaces('RopeOutroData', 'SCORE.cgf',                0,   9),
      loadSurfaces('RopeOutroData', 'hugo.cgf',                 0,   0),
    ]);

    [CaveResources.syncHugoStart, CaveResources.syncHugoDie] = await Promise.all([
      loadSync('RopeOutroData', '002-06.oos'),
      loadSync('RopeOutroData', '002-09.oos'),
    ]);

    CaveResources.herErVi        = loadSpeak('RopeOutroData', '002-05.wav');
    CaveResources.trappeSnak     = loadSpeak('RopeOutroData', '002-06.wav');
    CaveResources.nuKommerJeg    = loadSpeak('RopeOutroData', '002-07.wav');
    CaveResources.afskyliaSnak   = loadSpeak('RopeOutroData', '002-08.wav');
    CaveResources.hugoKatapult   = loadSpeak('RopeOutroData', '002-09.wav');
    CaveResources.hugoSkydUd     = loadSpeak('RopeOutroData', '002-10.wav');
    CaveResources.afskyliaSkyudUd = loadSpeak('RopeOutroData', '002-11.wav');
    CaveResources.hugolineTak    = loadSpeak('RopeOutroData', '002-12.wav');

    CaveResources.stemning     = loadSfx('RopeOutroData', 'BA-13.WAV');
    CaveResources.fodtrin1     = loadSfx('RopeOutroData', 'BA-15.WAV');
    CaveResources.fodtrin2     = loadSfx('RopeOutroData', 'BA-16.WAV');
    CaveResources.hivIReb      = loadSfx('RopeOutroData', 'BA-17.WAV');
    CaveResources.fjeder       = loadSfx('RopeOutroData', 'BA-18.WAV');
    CaveResources.prePuf       = loadSfx('RopeOutroData', 'BA-21.WAV');
    CaveResources.puf          = loadSfx('RopeOutroData', 'BA-22.WAV');
    CaveResources.tastTrykket  = loadSfx('RopeOutroData', 'BA-24.WAV');
    CaveResources.preFanfare   = loadSfx('RopeOutroData', 'BA-101.WAV');
    CaveResources.fanfare      = loadSfx('RopeOutroData', 'BA-102.WAV');
    CaveResources.fugleSkrig   = loadSfx('RopeOutroData', 'BA-104.WAV');
    CaveResources.trappeGrin   = loadSfx('RopeOutroData', 'HEXHAHA.WAV');
    CaveResources.skrig        = loadSfx('RopeOutroData', 'SKRIG.WAV');
    CaveResources.scoreCounter = loadSfx('RopeOutroData', 'COUNTER.WAV');
  }
}
