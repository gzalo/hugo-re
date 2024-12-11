from animation import Animation
from forest.forest_resources import ForestResources
from forest.hurt_rock_talking import HurtRockTalking
from state import State


class HurtRockHitAnimation(State):
    def process_events(self, events):
        if self.get_frame_index_fast() >= len(ForestResources.hit_rock):
            return HurtRockTalking(self.parent)

    def render(self, screen):
        screen.blit(Animation.get_frame(ForestResources.hit_rock, self.get_frame_index_fast()), (0,0))
        self.parent.render_bottom(screen)

