import pygame

from animation import Animation
from forest.forest_resources import ForestResources
from forest.hurt_branch_talking import HurtBranchTalking
from state import State


class HurtBranchAnimation(State):
    def process_events(self, events):
        if self.get_frame_index_fast() >= len(ForestResources.hugohitlog):
            return HurtBranchTalking(self.parent)

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(ForestResources.sfx_birds)

    def render(self, screen):
        screen.blit(Animation.get_frame(ForestResources.hugohitlog, self.get_frame_index_fast()), (0,0))
        self.parent.render_bottom(screen)
