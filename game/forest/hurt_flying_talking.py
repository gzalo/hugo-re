import pygame

from forest.forest_resources import ForestResources
from state import State


class HurtFlyingTalking(State):
    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.sync_catapult_talktop):
            return self.parent.reduce_lifes()

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(ForestResources.speak_catapult_talktop)

    def render(self, screen):
        screen.blit(ForestResources.catapult_airtalk[ForestResources.sync_catapult_talktop[self.get_frame_index()] - 1], (0,0))
