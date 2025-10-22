import pygame

from audio_helper import AudioHelper
from forest.forest_resources import ForestResources
from forest.playing import Playing
from state import State

class ScyllaButton(State):
    def process_events(self, events):
        if self.one_shot(0.5, "sfx_lightning_warning"):
            AudioHelper.play(ForestResources.sfx_lightning_warning, self.context.country)

        if self.get_state_time() > 2:
            return Playing

        return None

    def render(self, screen):
        images = [
            ForestResources.sculla_hand1[0],
            ForestResources.sculla_hand2[0]
        ]
        screen.blit(images[self.get_frame_index() % 2], (0,0))
