from game_data import GameData


class ForestUtils:

    @staticmethod
    def reduce_lives(context: GameData, obstacle_type: int = 0):
        context.forest_lives -= 1

        # Track which obstacle caused this life loss
        if context.forest_obstacles_hit is None:
            context.forest_obstacles_hit = []
        if obstacle_type > 0:
            context.forest_obstacles_hit.append(obstacle_type)

        if context.forest_lives == 0:
            from forest.talking_game_over import TalkingGameOver
            return TalkingGameOver
        else:
            from forest.talking_after_hurt import TalkingAfterHurt
            return TalkingAfterHurt
