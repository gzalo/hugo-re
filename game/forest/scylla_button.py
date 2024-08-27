import pygame

from forest.forest_resources import ForestResources
from forest.playing import Playing
from state import State

class ScyllaButton(State):
    def process_events(self, events):
        if self.one_shot(0.5, "sfx_lightning_warning"):
            pygame.mixer.Sound.play(ForestResources.sfx_lightning_warning)

        if self.get_state_time() > 2:
            return Playing(self.parent)

    def render(self, screen):
        images = [
            ForestResources.sculla_hand1[0],
            ForestResources.sculla_hand2[0]
        ]
        screen.blit(images[self.get_frame_index() % 2], (0,0))
