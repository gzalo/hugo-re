import pygame

from cave.cave_resources import CaveResources
from cave.going_rope import GoingRope
from config import Config
from phone_events import PhoneEvents
from state import State


class WaitingInput(State):
    def process_events(self, phone_events: PhoneEvents):
        if phone_events.press_3:
            return GoingRope(self.parent, 0)
        elif phone_events.press_6:
            return GoingRope(self.parent, 1)
        elif phone_events.press_9:
            return GoingRope(self.parent, 2)

        if self.one_shot(0.5, "afskylia_snak"):
            pygame.mixer.Sound.play(CaveResources.afskylia_snak)

        return None

    def render(self, screen):
        screen.blit(CaveResources.first_rope[0], (0, 0))

    def on_enter(self) -> None:
        super().on_enter()
        if not Config.ARGENTINE_VERSION:
            pygame.mixer.Sound.play(CaveResources.stemning)

    def on_exit(self) -> None:
        super().on_exit()
        pygame.mixer.Sound.play(CaveResources.tast_trykket)