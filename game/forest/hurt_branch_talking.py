import pygame

from animation import Animation
from forest.forest_resources import ForestResources
from forest.forest_utils import ForestUtils
from render_type import RenderType
from state import State


class HurtBranchTalking(State):
    def __init__(self, context):
        super().__init__(context)
        self.needs_bottom = RenderType.POST

    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.sync_hitlog):
            return ForestUtils.reduce_lives(self.context)
        return None

    def render(self, screen):
        screen.blit(Animation.get_sync_frame(ForestResources.hugohitlog_talk, ForestResources.sync_hitlog, self.get_frame_index()), (0, 0))

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(ForestResources.speak_hitlog)
