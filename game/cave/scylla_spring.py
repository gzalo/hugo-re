import pygame

from animation import Animation
from audio_helper import AudioHelper
from cave.cave_resources import CaveResources
from cave.family_cage_opens import FamilyCageOpens
from phone_events import PhoneEvents
from state import State


class ScyllaSpring(State):
    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(CaveResources.scylla_spring):
            return FamilyCageOpens

        if self.one_shot(1.5, "Fjeder"):
            AudioHelper.play(CaveResources.fjeder, self.context.audio_port)

        if self.one_shot(2, "AfskyliaSkydUd"):
            AudioHelper.play(CaveResources.afskylia_skyd_ud, self.context.audio_port)

        return None

    def render(self, screen):
        screen.blit(Animation.get_frame(CaveResources.scylla_spring, self.get_frame_index()), (0, 0))
