import pygame

from animation import Animation
from cave.cave_resources import CaveResources
from cave.climbing import Climbing
from config import Config
from phone_events import PhoneEvents
from state import State


class TalkingBeforeClimb(State):
    def process_events(self, phone_events: PhoneEvents):
        if not Config.ARGENTINE_VERSION and self.one_shot(4, "TrappeGrin"):
                pygame.mixer.Sound.play(CaveResources.trappe_grin)

        if self.get_frame_index() >= len(CaveResources.sync_hugo_start):
            return Climbing
        return None

    def render(self, screen):
        screen.blit(Animation.get_sync_frame(CaveResources.talks, CaveResources.sync_hugo_start, self.get_frame_index()), (0, 0))

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(CaveResources.trappe_snak)

