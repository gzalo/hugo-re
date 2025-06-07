from animation import Animation
from forest.forest_resources import ForestResources
from forest.hurt_rock_talking import HurtRockTalking
from render_type import RenderType
from state import State


class HurtRockHitAnimation(State):
    def __init__(self, context):
        super().__init__(context)
        self.needs_bottom = RenderType.POST

    def process_events(self, events):
        if self.get_frame_index_fast() >= len(ForestResources.hit_rock):
            return HurtRockTalking

        return None

    def render(self, screen):
        screen.blit(Animation.get_frame(ForestResources.hit_rock, self.get_frame_index_fast()), (0,0))

