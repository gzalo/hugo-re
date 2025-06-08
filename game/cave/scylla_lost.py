import pygame.mixer

from animation import Animation
from cave.cave_resources import CaveResources
from cave.family_cage_opens import FamilyCageOpens
from cave.scylla_spring import ScyllaSpring
from game_data import GameData
from phone_events import PhoneEvents
from state import State


class ScyllaLost(State):
    hugo_sprite_pos = [
        (25, 105),
        (97, 100),
        (172, 102),
    ]

    def __init__(self, context: GameData):
        super().__init__(context)
        self.animation = [CaveResources.scylla_bird, CaveResources.scylla_leaves, CaveResources.scylla_ropes][context.cave_win_type]

    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(self.animation):
            return ScyllaSpring if self.context.cave_win_type == 2 else FamilyCageOpens

        if self.context.cave_win_type == 0: # Bird
            if self.one_shot(0.5, "FugleSkrig"):
                pygame.mixer.Sound.play(CaveResources.fugle_skrig)
        elif self.context.cave_win_type == 1: # Leaves
            if self.one_shot(0.5, "Skrig"):
                pygame.mixer.Sound.play(CaveResources.skrig)
        else: # Ropes
            if self.one_shot(1, "PrePuf"):
                pygame.mixer.Sound.play(CaveResources.pre_puf)
            if self.one_shot(2, "Puf"):
                pygame.mixer.Sound.play(CaveResources.puf)
        return None


    def render(self, screen):
        screen.blit(Animation.get_frame(self.animation, self.get_frame_index()), (0,0))
        screen.blit(CaveResources.hugo[0], self.hugo_sprite_pos[self.context.cave_selected_rope])
