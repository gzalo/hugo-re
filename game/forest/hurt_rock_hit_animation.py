from forest.forest_resources import ForestResources
from forest.hurt_rock_talking import HurtRockTalking
from state import State


class HurtRockHitAnimation(State):
    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.hit_rock):
            return HurtRockTalking(self.parent)

    def render(self, screen):
        self.parent.render_bottom(screen)
        screen.blit(ForestResources.hit_rock[self.get_frame_index()], (0,0))