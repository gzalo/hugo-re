from phone_events import PhoneEvents
from tv_show.tv_show_resources import TvShowResources
from tv_show.video_state import VideoState


class Attract(VideoState):
    def __init__(self, parent):
        super().__init__(parent, TvShowResources.videos_attract[parent.country], True)

    def process_events(self, phone_events: PhoneEvents):
        super().process_events(phone_events)
        from tv_show.initial import Initial
        from tv_show.playing import Playing

        if phone_events.offhook:
            return Initial(self.parent)

        if phone_events.press_5:
            return Playing(self.parent)