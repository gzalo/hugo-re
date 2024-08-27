import random

import pygame

from cave.cave_resources import CaveResources
from cave.lost import Lost
from cave.scylla_lost import ScyllaLost
from phone_events import PhoneEvents
from state import State


class GoingRope(State):
    def __init__(self, parent, selected_rope):
        super().__init__(parent)
        self.animation = [CaveResources.first_rope, CaveResources.second_rope, CaveResources.third_rope][selected_rope]
        self.selected_rope = selected_rope

    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(self.animation):
            return self.calc_win_or_lose()

        sound_time = [2,3,4][self.selected_rope]
        if self.one_shot(sound_time, "HivIReb"):
            pygame.mixer.Sound.play(CaveResources.hiv_i_reb)

        if self.every(0.4, "Fodtrin1"):
            pygame.mixer.Sound.play(CaveResources.fodtrin1)

        if self.every(0.4, "Fodtrin2", 0.4):
            pygame.mixer.Sound.play(CaveResources.fodtrin2)

    def render(self, screen):
        screen.blit(self.animation[self.get_frame_index()], (0, 0))

    def calc_win_or_lose(self):
        random_value = random.randint(0, 3)
        if random_value == 0: # LOST
            return Lost(self.parent, self.selected_rope)
        elif random_value == 1: # WON BIRD
            self.parent.multiply_score(2)
            return ScyllaLost(self.parent, self.selected_rope, 0)
        elif random_value == 2:  # WON LEFT
            self.parent.multiply_score(3)
            return ScyllaLost(self.parent, self.selected_rope, 1)
        elif random_value == 3:  # WON ROPE
            self.parent.multiply_score(4)
            return ScyllaLost(self.parent, self.selected_rope, 2)


