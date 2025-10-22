import pygame.mixer

from animation import Animation
from audio_helper import AudioHelper
from forest.forest_resources import ForestResources
from forest.hurt_flying_falling_hang_animation import HurtFlyingFallingHangAnimation
from state import State


class HurtFlyingFalling(State):
    def process_events(self, events):
        if self.get_frame_index_fast() >= len(ForestResources.catapult_fall):
            return HurtFlyingFallingHangAnimation
        return None

    def on_enter(self) -> None:
        super().on_enter()
        AudioHelper.play(ForestResources.speak_catapult_down, self.context.audio_port)
        AudioHelper.play(ForestResources.sfx_hugo_crash, self.context.audio_port)

    def render(self, screen):
        screen.blit(Animation.get_frame(ForestResources.catapult_fall, self.get_frame_index_fast()), (0,0))



