import pygame

from animation import Animation
from audio_helper import AudioHelper
from cave.cave_resources import CaveResources
from null_state import NullState
from phone_events import PhoneEvents
from state import State


class FamilyHappy(State):
    def render(self, screen):
        screen.blit(Animation.get_frame(CaveResources.happy, self.get_frame_index()), (0, 0))

    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(CaveResources.happy):
            return NullState
        return None

    def on_enter(self) -> None:
        super().on_enter()
        AudioHelper.play(CaveResources.fanfare, self.context.country)

    def on_exit(self) -> None:
        super().on_exit()
        AudioHelper.stop(CaveResources.fanfare, self.context.country)

