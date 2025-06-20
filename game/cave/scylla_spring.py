import pygame

from animation import Animation
from cave.cave_resources import CaveResources
from cave.family_cage_opens import FamilyCageOpens
from phone_events import PhoneEvents
from state import State


class ScyllaSpring(State):
    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(CaveResources.scylla_spring):
            return FamilyCageOpens

        if self.one_shot(1.5, "Fjeder"):
            pygame.mixer.Sound.play(CaveResources.fjeder)

        if self.one_shot(2, "AfskyliaSkydUd"):
            pygame.mixer.Sound.play(CaveResources.afskylia_skyd_ud)

        return None

    def render(self, screen):
        screen.blit(Animation.get_frame(CaveResources.scylla_spring, self.get_frame_index()), (0, 0))
