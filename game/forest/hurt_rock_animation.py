from animation import Animation
from forest.forest_resources import ForestResources
from forest.hurt_rock_hit_animation import HurtRockHitAnimation
from game_data import GameData
from render_type import RenderType
from state import State


class HurtRockAnimation(State):
    def __init__(self, context: GameData):
        super().__init__(context)
        self.needs_bottom = RenderType.POST

    def process_events(self, events):
        if self.get_frame_index_fast() >= len(ForestResources.hugo_lookrock):
            return HurtRockHitAnimation

        return None

    def render(self, screen):
        screen.blit(Animation.get_frame(ForestResources.hugo_lookrock, self.get_frame_index_fast()), (0,0))

