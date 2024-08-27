import pygame

from forest.forest_resources import ForestResources
from forest.playing import Playing
from state import State


class WaitIntro(State):
    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.sync_start):
            return Playing(self.parent)

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(ForestResources.speak_start)

    def render(self, screen):
        self.parent.render_background(screen)
        self.parent.render_bottom(screen)
        screen.blit(ForestResources.hugo_telllives[ForestResources.sync_start[self.get_frame_index()]], (128, -16))
