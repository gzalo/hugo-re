from phone_events import PhoneEvents
from tv_show.press_5 import Press5
from tv_show.tv_show_resources import TvShowResources
from tv_show.video_state import VideoState


class Initial(VideoState):
    def __init__(self, parent):
        super().__init__(parent, TvShowResources.videos_initial[parent.country])

    def process_events(self, phone_events: PhoneEvents):
        super().process_events(phone_events)

        from tv_show.attract import Attract

        if phone_events.hungup:
            return Attract(self.parent)

        if self.video_ended():
            return Press5(self.parent)
