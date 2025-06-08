from game_data import GameData
from phone_events import PhoneEvents
from tv_show.attract import Attract
from tv_show.in_cave import InCave
from tv_show.tv_show_resources import TvShowResources
from tv_show.video_state import VideoState


class GoingCave(VideoState):
    def __init__(self, context: GameData):
        super().__init__(context, TvShowResources.videos_going_scylla[context.country])

    def process_events(self, phone_events: PhoneEvents):
        super().process_events(phone_events)

        if phone_events.hungup:
            return Attract

        if self.video_ended():
            return InCave

        return None

