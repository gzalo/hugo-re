import pygame.mixer

from animation import Animation
from audio_helper import AudioHelper
from cave.cave_resources import CaveResources
from null_state import NullState
from phone_events import PhoneEvents
from state import State
from config import Config


class LostSpring(State):
    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(CaveResources.hugo_spring):
            return NullState

        if self.one_shot(2.5, "HugoSkydUd"):
            AudioHelper.play(CaveResources.hugo_skyd_ud, self.context.audio_port)

        return None

    def render(self, screen):
        screen.blit(Animation.get_frame(CaveResources.hugo_spring, self.get_frame_index()), (0, 0))

    def on_enter(self) -> None:
        super().on_enter()
        AudioHelper.play(CaveResources.hugo_katapult, self.context.audio_port)

    def on_exit(self) -> None:
        super().on_exit()
        if Config.ARGENTINE_VERSION:
            AudioHelper.fadeout(self.context.cave_bg_music_id, self.context.audio_port, 1)
        else:
            AudioHelper.fadeout(self.context.cave_stemning_id, self.context.audio_port, 1)