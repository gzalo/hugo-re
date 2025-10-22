import pygame

from animation import Animation
from audio_helper import AudioHelper
from forest.forest_resources import ForestResources
from forest.hurt_flying_falling import HurtFlyingFalling
from state import State


class HurtFlyingTalking(State):
    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.sync_catapult_talktop):
            return HurtFlyingFalling

        return None

    def on_enter(self) -> None:
        super().on_enter()
        AudioHelper.play(ForestResources.speak_catapult_talktop, self.context.country)

    def render(self, screen):
        screen.blit(Animation.get_sync_frame(ForestResources.catapult_airtalk, ForestResources.sync_catapult_talktop, self.get_frame_index()), (0,0))

