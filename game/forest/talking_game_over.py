import pygame

from animation import Animation
from audio_helper import AudioHelper
from forest.forest_resources import ForestResources
from game_data import GameData
from null_state import NullState
from render_type import RenderType
from state import State


class TalkingGameOver(State):
    def __init__(self, context: GameData):
        super().__init__(context)
        self.needs_background = RenderType.PRE
        self.needs_bottom = RenderType.PRE

    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.sync_gameover):
            return NullState
        return None

    def on_enter(self) -> None:
        super().on_enter()
        AudioHelper.play(ForestResources.speak_gameover, self.context.audio_port)

    def render(self, screen):
        screen.blit(Animation.get_sync_frame(ForestResources.hugo_telllives, ForestResources.sync_gameover, self.get_frame_index()), (128,-16))
