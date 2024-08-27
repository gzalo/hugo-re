import pygame

from cave.cave_resources import CaveResources
from cave.talking_before_climb import TalkingBeforeClimb
from config import Config
from phone_events import PhoneEvents
from state import State

class WaitingBeforeTalking(State):

    def process_events(self, phone_events: PhoneEvents) -> State:
        if self.get_state_time() > 2.5:
            return TalkingBeforeClimb(self.parent)

    def render(self, screen):
        screen.blit(CaveResources.talks[12], (0,0))

    def on_enter(self) -> None:
        super().on_enter()
        pygame.mixer.Sound.play(CaveResources.her_er_vi)

        if Config.ARGENTINE_VERSION:
            pygame.mixer.Sound.play(CaveResources.bg_music)
        else:
            pygame.mixer.Sound.play(CaveResources.stemning)
