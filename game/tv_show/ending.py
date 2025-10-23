from game_data import GameData
from phone_events import PhoneEvents
from tv_show.tv_show_resources import TvShowResources
from tv_show.video_state import VideoState


class Ending(VideoState):
    def __init__(self, context: GameData):
        super().__init__(context, TvShowResources.videos_ending[context.country], False, TvShowResources.audio_ending[context.country])

    def process_events(self, phone_events: PhoneEvents):
        super().process_events(phone_events)
        from tv_show.attract import Attract

        if phone_events.hungup:
            return Attract

        return None