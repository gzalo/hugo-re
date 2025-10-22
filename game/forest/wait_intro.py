import pygame

from animation import Animation
from audio_helper import AudioHelper
from forest.forest_resources import ForestResources
from forest.playing import Playing
from game_data import GameData
from render_type import RenderType
from state import State


class WaitIntro(State):
    def __init__(self, context: GameData):
        super().__init__(context)
        self.needs_background = RenderType.PRE
        self.needs_bottom = RenderType.PRE

    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.sync_start):
            return Playing
        return None

    def on_enter(self) -> None:
        super().on_enter()
        AudioHelper.play(ForestResources.speak_start, self.context.audio_port)

    def render(self, screen):
        screen.blit(Animation.get_sync_frame(ForestResources.hugo_telllives, ForestResources.sync_start, self.get_frame_index()), (128, -16))
