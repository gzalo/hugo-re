import pygame

from forest.forest_resources import ForestResources
from state import State


class HurtBranchTalking(State):
    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.sync_hitlog):
            return self.parent.reduce_lifes()

    def render(self, screen):
        screen.blit(ForestResources.hugohitlog_talk[ForestResources.sync_hitlog[self.get_frame_index()]-1], (0, 0))
        self.parent.render_bottom(screen)

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(ForestResources.speak_hitlog)
