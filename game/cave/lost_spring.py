import pygame.mixer

from animation import Animation
from cave.cave_resources import CaveResources
from null_state import NullState
from phone_events import PhoneEvents
from state import State


class LostSpring(State):
    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(CaveResources.hugo_spring):
            return NullState

        if self.one_shot(2.5, "HugoSkydUd"):
            pygame.mixer.Sound.play(CaveResources.hugo_skyd_ud)

        return None

    def render(self, screen):
        screen.blit(Animation.get_frame(CaveResources.hugo_spring, self.get_frame_index()), (0, 0))

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(CaveResources.hugo_katapult)
