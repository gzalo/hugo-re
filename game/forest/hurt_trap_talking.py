import pygame

from animation import Animation
from forest.forest_resources import ForestResources
from forest.forest_utils import ForestUtils
from state import State


class HurtTrapTalking(State):
    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.sync_trap):
            return ForestUtils.reduce_lives(self.context)

        return None

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(ForestResources.speak_trap)

    def render(self, screen):
        screen.blit(Animation.get_sync_frame(ForestResources.hugo_traptalk, ForestResources.sync_trap, self.get_frame_index()), (0,0))
