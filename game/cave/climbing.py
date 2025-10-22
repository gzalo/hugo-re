import pygame

from animation import Animation
from audio_helper import AudioHelper
from cave.cave_resources import CaveResources
from cave.waiting_input import WaitingInput
from phone_events import PhoneEvents
from state import State


class Climbing(State):
    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(CaveResources.climbs):
            return WaitingInput

        if self.one_shot(1, "NuKommerJeg"):
            AudioHelper.play(CaveResources.nu_kommer_jeg, self.context.country)

        if self.one_shot(2, "PreFanfare"):
            AudioHelper.play(CaveResources.pre_fanfare, self.context.country)

        return None

    def render(self, screen):
        screen.blit(Animation.get_frame(CaveResources.climbs, self.get_frame_index()), (0, 0))
