import pygame

from forest.forest_resources import ForestResources
from null_state import NullState
from state import State


class WinTalking(State):
    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.sync_levelcompleted):
            return NullState(self.parent)

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(ForestResources.speak_levelcompleted)

    def render(self, screen):
        self.parent.render_background(screen)
        self.parent.render_bottom(screen)
        screen.blit(ForestResources.hugo_telllives[ForestResources.sync_levelcompleted[self.get_frame_index()]], (128, -16))
