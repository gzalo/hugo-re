import pygame.mixer

from animation import Animation
from forest.forest_resources import ForestResources
from forest.hurt_flying_falling_hang_animation import HurtFlyingFallingHangAnimation
from state import State


class HurtFlyingFalling(State):
    def process_events(self, events):
        if self.get_frame_index_fast() >= len(ForestResources.catapult_fall):
            return HurtFlyingFallingHangAnimation(self.parent)

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(ForestResources.speak_catapult_down)
        pygame.mixer.Sound.play(ForestResources.sfx_hugo_crash)

    def render(self, screen):
        screen.blit(Animation.get_frame(ForestResources.catapult_fall, self.get_frame_index_fast()), (0,0))



