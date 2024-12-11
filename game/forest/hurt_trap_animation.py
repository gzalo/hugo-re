from animation import Animation
from forest.forest_resources import ForestResources
from forest.hurt_trap_talking import HurtTrapTalking
from state import State


class HurtTrapAnimation(State):
    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.hugo_traphurt):
            return HurtTrapTalking(self.parent)

    def render(self, screen):
        screen.blit(Animation.get_frame(ForestResources.hugo_traphurt, self.get_frame_index()), (0,0))

