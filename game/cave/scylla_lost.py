import pygame.mixer

from cave.cave_resources import CaveResources
from cave.family_cage_opens import FamilyCageOpens
from cave.scylla_spring import ScyllaSpring
from phone_events import PhoneEvents
from state import State


class ScyllaLost(State):
    hugo_sprite_pos = [
        (25, 105),
        (97, 100),
        (172, 102),
    ]

    def __init__(self, parent, selected_rope, lost_type):
        super().__init__(parent)
        self.lost_type = lost_type
        self.animation = [CaveResources.scylla_bird, CaveResources.scylla_leaves, CaveResources.scylla_ropes][lost_type]
        self.selected_rope = selected_rope

    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(self.animation):
            return ScyllaSpring(self.parent) if self.lost_type == 2 else FamilyCageOpens(self.parent)

        if self.lost_type == 0: # Bird
            if self.one_shot(0.5, "FugleSkrig"):
                pygame.mixer.Sound.play(CaveResources.fugle_skrig)
        elif self.lost_type == 1: # Leaves
            if self.one_shot(0.5, "Skrig"):
                pygame.mixer.Sound.play(CaveResources.skrig)
        else: # Ropes
            if self.one_shot(1, "PrePuf"):
                pygame.mixer.Sound.play(CaveResources.pre_puf)
            if self.one_shot(2, "Puf"):
                pygame.mixer.Sound.play(CaveResources.puf)

    def render(self, screen):
        screen.blit(self.animation[self.get_frame_index()], (0,0))
        screen.blit(CaveResources.hugo[0], self.hugo_sprite_pos[self.selected_rope])

