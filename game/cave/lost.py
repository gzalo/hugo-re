import pygame

from animation import Animation
from cave.cave_resources import CaveResources
from cave.lost_spring import LostSpring
from game_data import GameData
from phone_events import PhoneEvents
from state import State


class Lost(State):
    def __init__(self, context: GameData):
        super().__init__(context)
        self.animation = [
            CaveResources.hugo_puff_first,
            CaveResources.hugo_puff_second,
            CaveResources.hugo_puff_third
        ][context.cave_selected_rope]

    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(self.animation):
            return LostSpring

        if self.one_shot(1, "PrePuf"):
            pygame.mixer.Sound.play(CaveResources.pre_puf)

        if self.one_shot(2, "Puf"):
            pygame.mixer.Sound.play(CaveResources.puf)

        return None

    def render(self, screen):
        screen.blit(Animation.get_frame(self.animation, self.get_frame_index()), (0, 0))
