import pygame

from animation import Animation
from audio_helper import AudioHelper
from forest.forest_resources import ForestResources
from forest.forest_utils import ForestUtils
from game_data import GameData
from render_type import RenderType
from state import State


class HurtRockTalking(State):
    def __init__(self, context: GameData):
        super().__init__(context)
        self.needs_bottom = RenderType.POST

    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.sync_rock):
            return ForestUtils.reduce_lives(self.context)

        return None

    def on_enter(self) -> None:
        super().on_enter()
        AudioHelper.play(ForestResources.speak_rock, self.context.country)

    def render(self, screen):
        screen.blit(Animation.get_sync_frame(ForestResources.hit_rock_sync, ForestResources.sync_rock, self.get_frame_index()), (0,0))

