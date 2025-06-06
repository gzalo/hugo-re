from config import Config
from phone_events import PhoneEvents
from state import State
from tv_show.attract import Attract
from tv_show.playing import Playing
from tv_show.tv_show_resources import TvShowResources


class Instructions(State):
    def process_events(self, phone_events: PhoneEvents):
        if phone_events.hungup:
            return Attract

        if self.get_state_time() > Config.INSTRUCTIONS_TIMEOUT:
            return Playing

        return None


    def render(self, screen):
        screen.blit(TvShowResources.instructions["Forest"], (0, 0))

