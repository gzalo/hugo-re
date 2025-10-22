import pygame

from audio_helper import AudioHelper
from cave.cave_resources import CaveResources
from cave.talking_before_climb import TalkingBeforeClimb
from config import Config
from phone_events import PhoneEvents
from state import State

class WaitingBeforeTalking(State):
    def process_events(self, phone_events: PhoneEvents):
        if self.get_state_time() > 2.5:
            return TalkingBeforeClimb
        return None

    def render(self, screen):
        screen.blit(CaveResources.talks[12], (0,0))

    def on_enter(self) -> None:
        super().on_enter()
        AudioHelper.play(CaveResources.her_er_vi, self.context.audio_port)

        if Config.ARGENTINE_VERSION:
            AudioHelper.play(CaveResources.bg_music, self.context.audio_port)
        else:
            AudioHelper.play(CaveResources.stemning, self.context.audio_port)
