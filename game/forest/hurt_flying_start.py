import pygame

from animation import Animation
from audio_helper import AudioHelper
from forest.forest_resources import ForestResources
from forest.hurt_flying_talking import HurtFlyingTalking
from state import State


class HurtFlyingStart(State):
    def process_events(self, events):
        if self.get_frame_index_fast() >= len(ForestResources.catapult_fly):
            return HurtFlyingTalking

        if self.one_shot(2.7, "crash"):
            AudioHelper.play(ForestResources.sfx_hugo_screenklir, self.context.country)

        if self.one_shot(2.7, "uy"):
            AudioHelper.play(ForestResources.speak_catapult_hit, self.context.country)

        return None

    def on_enter(self) -> None:
        super().on_enter()
        AudioHelper.play(ForestResources.speak_catapult_up, self.context.country)

    def render(self, screen):
        screen.blit(Animation.get_frame(ForestResources.catapult_fly, self.get_frame_index_fast()), (0,0))

