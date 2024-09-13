import random

from config import Config
from phone_events import PhoneEvents
from tv_show.attract import Attract
from tv_show.playing import Playing


class TvShowParent:

    def set_random_game(self):
        if len(Config.GAMES) == 1:
            self.current_game = next(iter(Config.GAMES))
            return
        new_game = random.choice(list(Config.GAMES.keys()))
        while new_game == self.current_game:
            new_game = random.choice(list(Config.GAMES.keys()))
        self.current_game = new_game

    def __init__(self, country, parent):
        self.country = country
        self.pre_cave_score = 0
        self._state = Attract(self)
        self._state.on_enter()
        self.current_game = next(iter(Config.GAMES))
        self.parent = parent

    def handle_events(self, phone_events: PhoneEvents):
        next_state = self._state.process_events(phone_events)

        if next_state is not None:
            self._state.on_exit()
            self._state = next_state
            self._state.on_enter()

    def render(self, screen):
        self._state.render(screen)

    def is_playing(self):
        return not isinstance(self._state, Attract)

    def external_effect(self, effect):
        if isinstance(self._state, Playing):
            self._state.external_effect(effect)