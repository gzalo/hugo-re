import pygame

from forest.forest_resources import ForestResources
from forest.hurt_flying_talking import HurtFlyingTalking
from state import State


class HurtFlyingStart(State):
    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.catapult_fly):
            return HurtFlyingTalking(self.parent)

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(ForestResources.speak_catapult_up)

    def render(self, screen):
        screen.blit(ForestResources.catapult_fly[self.get_frame_index()], (0,0))

