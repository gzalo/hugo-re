from forest.forest_resources import ForestResources
from forest.hurt_rock_hit_animation import HurtRockHitAnimation
from state import State


class HurtRockAnimation(State):
    def process_events(self, events):
        if self.get_frame_index_fast() >= len(ForestResources.hugo_lookrock):
            return HurtRockHitAnimation(self.parent)

    def render(self, screen):
        screen.blit(ForestResources.hugo_lookrock[self.get_frame_index_fast()], (0,0))
        self.parent.render_bottom(screen)

