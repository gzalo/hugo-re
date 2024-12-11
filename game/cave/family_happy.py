import pygame

from animation import Animation
from cave.cave_resources import CaveResources
from null_state import NullState
from phone_events import PhoneEvents
from state import State


class FamilyHappy(State):
    def render(self, screen):
        screen.blit(Animation.get_frame(CaveResources.happy, self.get_frame_index()), (0, 0))

    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(CaveResources.happy):
            return NullState(self.parent)

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(CaveResources.fanfare)

    def on_exit(self) -> None:
        super().on_exit()
        pygame.mixer.Sound.stop(CaveResources.fanfare)

