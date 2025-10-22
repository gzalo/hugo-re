import random

import pygame

from animation import Animation
from audio_helper import AudioHelper
from cave.cave_resources import CaveResources
from cave.lost import Lost
from cave.scylla_lost import ScyllaLost
from game_data import GameData
from phone_events import PhoneEvents
from state import State


class GoingRope(State):
    def __init__(self, context: GameData):
        super().__init__(context)
        self.animation = [CaveResources.first_rope, CaveResources.second_rope, CaveResources.third_rope][context.cave_selected_rope]

    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(self.animation):
            return self.calc_win_or_lose()

        sound_time = [2,3,4][self.context.cave_selected_rope]
        if self.one_shot(sound_time, "HivIReb"):
            AudioHelper.play(CaveResources.hiv_i_reb, self.context.country)

        if self.every(0.4, "Fodtrin1"):
            AudioHelper.play(CaveResources.fodtrin1, self.context.country)

        if self.every(0.4, "Fodtrin2", 0.4):
            AudioHelper.play(CaveResources.fodtrin2, self.context.country)

        return None

    def render(self, screen):
        screen.blit(Animation.get_frame(self.animation, self.get_frame_index()), (0, 0))

    def calc_win_or_lose(self):
        random_value = random.randint(0, 3)
        if random_value == 0: # LOST
            return Lost
        elif random_value == 1: # WON BIRD
            self.context.cave_win_type = 0
            self.context.forest_score *= 2
            return ScyllaLost
        elif random_value == 2:  # WON LEFT
            self.context.cave_win_type = 1
            self.context.forest_score *= 3
            return ScyllaLost
        elif random_value == 3:  # WON ROPE
            self.context.cave_win_type = 2
            self.context.forest_score *= 4
            return ScyllaLost
        else:
            return None



