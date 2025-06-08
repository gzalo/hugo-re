from forest.forest_game import ForestGame
from game_data import GameData
from phone_events import PhoneEvents
from state import State
from tv_show.attract import Attract
from tv_show.going_cave import GoingCave


class Playing(State):
    def __init__(self, context: GameData):
        super().__init__(context)
        self.forest = ForestGame(context)

    def process_events(self, phone_events: PhoneEvents):
        if phone_events.hungup:
            return Attract

        if self.forest.ended:
            return GoingCave

        return self.forest.process_events(phone_events)

    def render(self, screen):
        self.forest.render(screen)

    def on_exit(self):
        super().on_exit()
        self.forest = None

    def external_effect(self, effect):
        if self.forest is not None:
            self.forest.external_effect(effect)
