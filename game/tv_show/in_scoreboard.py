from game_data import GameData
from phone_events import PhoneEvents
from scoreboard.scoreboard_game import ScoreboardGame
from state import State
from tv_show.attract import Attract
from tv_show.going_cave import GoingCave


class InScoreboard(State):
    def __init__(self, context: GameData):
        super().__init__(context)
        self.scoreboard = ScoreboardGame(context)

    def process_events(self, phone_events: PhoneEvents):
        if phone_events.hungup:
            return Attract

        if self.scoreboard.ended:
            return GoingCave

        self.scoreboard.process_events(phone_events)
        return None

    def render(self, screen):
        self.scoreboard.render(screen)

    def on_exit(self):
        super().on_exit()
        if self.scoreboard is not None and not self.scoreboard.ended:
            self.scoreboard.end()
        self.scoreboard = None
