import pygame

from cave.cave_resources import CaveResources
from cave.waiting_input import WaitingInput
from phone_events import PhoneEvents
from state import State


class Climbing(State):
    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(CaveResources.climbs):
            return WaitingInput(self.parent)

        if self.one_shot(1, "NuKommerJeg"):
            pygame.mixer.Sound.play(CaveResources.nu_kommer_jeg)

        if self.one_shot(2, "PreFanfare"):
            pygame.mixer.Sound.play(CaveResources.pre_fanfare)

    def render(self, screen):
        screen.blit(CaveResources.climbs[self.get_frame_index()], (0, 0))