import pygame

from forest.forest_resources import ForestResources
from state import State


class HurtRockTalking(State):
    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.sync_rock):
            return self.parent.reduce_lifes()

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(ForestResources.speak_rock)

    def render(self, screen):
        screen.blit(ForestResources.hit_rock_sync[ForestResources.sync_rock[self.get_frame_index()]-1], (0,0))
        self.parent.render_bottom(screen)

