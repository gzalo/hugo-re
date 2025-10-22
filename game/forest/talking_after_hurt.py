import pygame

from animation import Animation
from audio_helper import AudioHelper
from forest.forest_resources import ForestResources
from forest.playing import Playing
from game_data import GameData
from render_type import RenderType
from state import State


class TalkingAfterHurt(State):
    def __init__(self, context: GameData):
        super().__init__(context)
        self.needs_background = RenderType.PRE
        self.needs_bottom = RenderType.PRE

    def process_events(self, events):
        if (self.context.forest_lives == 1 and self.get_frame_index() >= len(ForestResources.sync_lastlife)) or \
            (self.context.forest_lives >= 1 and self.get_frame_index() >= len(ForestResources.sync_dieonce)):
            return Playing

        if self.one_shot(0.5, "knock1"):
            AudioHelper.play(ForestResources.sfx_hugo_knock, self.context.audio_port)

        if self.one_shot(0, "speak"):
            AudioHelper.play(ForestResources.sfx_hugo_knock, self.context.audio_port)

            if self.context.forest_lives == 1:
                AudioHelper.play(ForestResources.speak_lastlife, self.context.audio_port)
            else:
                AudioHelper.play(ForestResources.speak_dieonce, self.context.audio_port)

        return None

    def render(self, screen):
        sync = ForestResources.sync_lastlife if self.context.forest_lives == 1 else ForestResources.sync_dieonce
        screen.blit(Animation.get_sync_frame(ForestResources.hugo_telllives, sync, self.get_frame_index()), (128, -16))

        if self.get_frame_index() < 8 and self.get_frame_index() % 4 == 0:
            screen.blit(ForestResources.hugo_hand2[0], (96, 78))
        else:
            screen.blit(ForestResources.hugo_hand1[0], (96, 83))

