import pygame

from animation import Animation
from audio_helper import AudioHelper
from cave.cave_resources import CaveResources
from cave.family_happy import FamilyHappy
from config import Config
from phone_events import PhoneEvents
from state import State


class FamilyCageOpens(State):
    def on_exit(self) -> None:
        super().on_exit()
        if Config.ARGENTINE_VERSION:
            AudioHelper.fadeout(self.context.cave_bg_music_id, self.context.audio_port, 1000)
        else:
            AudioHelper.fadeout(self.context.cave_stemning_id, self.context.audio_port, 1000)

    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(CaveResources.family_cage):
            return FamilyHappy

        if self.one_shot(0.5, "HivIReb"):
            AudioHelper.play(CaveResources.hiv_i_reb, self.context.audio_port)

        if self.one_shot(1, "HugolineTak"):
            AudioHelper.play(CaveResources.hugoline_tak, self.context.audio_port)

        return None

    def render(self, screen):
        screen.blit(Animation.get_frame(CaveResources.family_cage, self.get_frame_index()), (0, 0))
