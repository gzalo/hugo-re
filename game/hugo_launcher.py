import random
import subprocess
import time

class HugoLauncher:
    current_game = "Forest"
    score = 0

    game_options = [
        "Plane",
        "Forest",
        "IceCavern",
        "SkateBoard",
        "Scuba",
        "Train"
    ]

    def start(self):
        self.score = 0

    def end(self):
        self.score = 1000

    def process(self):
        return False

    def set_random_game(self):
        new_game = random.choice(self.game_options)
        while new_game == self.current_game:
            new_game = random.choice(self.game_options)
        self.current_game = new_game

    def get_game(self):
        return self.current_game

    def get_games(self):
        return self.game_options

    def get_score(self):
        return self.score
