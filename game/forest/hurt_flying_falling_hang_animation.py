import pygame

from animation import Animation
from audio_helper import AudioHelper
from forest.forest_resources import ForestResources
from forest.hurt_flying_falling_hang_talking import HurtFlyingFallingHangTalking
from state import State


class HurtFlyingFallingHangAnimation(State):
    def process_events(self, events):
        if self.get_frame_index_fast() >= len(ForestResources.catapult_hang):
            return HurtFlyingFallingHangTalking
        return None

    def on_enter(self) -> None:
        super().on_enter()
        AudioHelper.play(ForestResources.sfx_hugo_hangstart, self.context.country)

    def render(self, screen):
        screen.blit(Animation.get_frame(ForestResources.catapult_hang, self.get_frame_index_fast()), (0,0))


