import pygame

from animation import Animation
from audio_helper import AudioHelper
from forest.forest_resources import ForestResources
from forest.hurt_branch_talking import HurtBranchTalking
from game_data import GameData
from render_type import RenderType
from state import State


class HurtBranchAnimation(State):
    def __init__(self, context: GameData):
        super().__init__(context)
        self.needs_bottom = RenderType.POST

    def process_events(self, events):
        if self.get_frame_index_fast() >= len(ForestResources.hugohitlog):
            return HurtBranchTalking
        else:
            return None

    def on_enter(self) -> None:
        super().on_enter()
        AudioHelper.play(ForestResources.sfx_birds, self.context.audio_port)

    def render(self, screen):
        screen.blit(Animation.get_frame(ForestResources.hugohitlog, self.get_frame_index_fast()), (0,0))
