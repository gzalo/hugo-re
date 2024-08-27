from cave.cave_game import CaveGame
from phone_events import PhoneEvents
from state import State
from tv_show.attract import Attract
from tv_show.ending import Ending


class InCave(State):
    def __init__(self, parent):
        super().__init__(parent)
        self.cave = CaveGame(parent.pre_cave_score)

    def process_events(self, phone_events: PhoneEvents):
        if phone_events.hungup:
            return Attract(self.parent)

        if self.cave.ended:
            return Ending(self.parent)

        self.cave.process_events(phone_events)

    def render(self, screen):
        self.cave.render(screen)

    def on_exit(self):
        super().on_exit()
        self.cave = None
