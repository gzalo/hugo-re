import pygame

from animation import Animation
from cave.cave_resources import CaveResources
from cave.family_happy import FamilyHappy
from config import Config
from phone_events import PhoneEvents
from state import State


class FamilyCageOpens(State):
    def on_exit(self) -> None:
        super().on_exit()
        if Config.ARGENTINE_VERSION:
            pygame.mixer.Sound.fadeout(CaveResources.bg_music, 1000)
        else:
            pygame.mixer.Sound.fadeout(CaveResources.stemning, 1000)

    def process_events(self, phone_events: PhoneEvents):
        if self.get_frame_index() >= len(CaveResources.family_cage):
            return FamilyHappy(self.parent)

        if self.one_shot(0.5, "HivIReb"):
            pygame.mixer.Sound.play(CaveResources.hiv_i_reb)

        if self.one_shot(1, "HugolineTak"):
            pygame.mixer.Sound.play(CaveResources.hugoline_tak)

    def render(self, screen):
        screen.blit(Animation.get_frame(CaveResources.family_cage, self.get_frame_index()), (0, 0))
