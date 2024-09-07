from phone_events import PhoneEvents
from tv_show.attract import Attract
from tv_show.have_luck import HaveLuck
from tv_show.tv_show_resources import TvShowResources
from tv_show.video_state import VideoState


class Press5(VideoState):
    def __init__(self, parent):
        super().__init__(parent, TvShowResources.videos_press_5[parent.country], True)

    def process_events(self, phone_events: PhoneEvents):
        super().process_events(phone_events)
        if phone_events.hungup:
            return Attract(self.parent)

        if phone_events.press_5:
            return HaveLuck(self.parent)

    def render(self, screen):
        super().render(screen)

        if self.has_looped:
            screen.blit(TvShowResources.press_5, (0, 0))