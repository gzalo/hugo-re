from game_data import GameData
from phone_events import PhoneEvents
from tv_show.tv_show_resources import TvShowResources
from tv_show.video_state import VideoState


class Attract(VideoState):
    def __init__(self, context: GameData):
        super().__init__(context, TvShowResources.videos_attract[context.country], True)

    def process_events(self, phone_events: PhoneEvents):
        super().process_events(phone_events)
        from tv_show.initial import Initial
        from tv_show.playing import Playing
        from tv_show.in_cave import InCave

        if phone_events.offhook:
            return Initial

        if phone_events.press_5:
            return Playing
        if phone_events.press_6:
            self.context.forest_score = 1000
            return InCave

        return None
