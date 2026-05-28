/** Port of game/forest/forest_utils.py */

import type { GameData } from '../gameData';
import type { StateClass } from '../state';
import { TalkingAfterHurt } from './talkingAfterHurt';
import { TalkingGameOver } from './talkingGameOver';

export class ForestUtils {
  static reduceLives(context: GameData, obstacleType: number = 0): StateClass {
    context.forestLives -= 1;

    if (obstacleType > 0) {
      context.forestObstaclesHit.push(String(obstacleType));
    }

    if (context.forestLives === 0) {
      return TalkingGameOver;
    } else {
      return TalkingAfterHurt;
    }
  }
}
