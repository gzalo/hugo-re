from game_data import GameData


class ForestUtils:

    @staticmethod
    def reduce_lives(context: GameData):
        context.forest_lives -= 1
        if context.forest_lives == 0:
            from forest.talking_game_over import TalkingGameOver
            return TalkingGameOver
        else:
            from forest.talking_after_hurt import TalkingAfterHurt
            return TalkingAfterHurt
