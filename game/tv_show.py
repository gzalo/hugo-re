from pyvidplayer2 import Video
from enum import Enum
import time
import random

from cave import Cave
from forest import Forest
from phone_events import PhoneEvents


class TvShowState(Enum):
    ATTRACT = 0
    INITIAL = 1
    PRESS_5 = 2
    HAVE_LUCK = 3
    INSTRUCTIONS = 4
    PLAYING_HUGO = 5
    GOING_SCYLLA = 6
    CAVE = 7
    ENDING = 8

class TvShow:
    INSTRUCTIONS_TIMEOUT = 3

    def switch_to(self, new_state: TvShowState | None):
        for video_key, video in self.videos.items():
            video.stop()
        if new_state is not None and new_state in self.videos:
            self.videos[new_state].restart()
        self.state = new_state
        self.state_start = time.time()

    def reloop(self):
        if self.state in self.videos and not self.videos[self.state].active:
            self.videos[self.state].restart()

    def has_ended(self):
        return self.state in self.videos and not self.videos[self.state].active

    def state_timeout(self, timeout):
        return time.time() - self.state_start > timeout

    def reset_state_timeout(self):
        self.state_start = time.time()

    def set_random_game(self):
        if len(self.games) == 1:
            self.current_game = next(iter(self.games))
            return
        new_game = random.choice(list(self.games.keys()))
        while new_game == self.current_game:
            new_game = random.choice(list(self.games.keys()))
        self.current_game = new_game

    def __init__(self, games, country):
        self.games = games

        prefix = f"videos/{country}/"
        self.videos = {
            TvShowState.ATTRACT: Video(prefix + "attract_demo.mp4"),
            TvShowState.INITIAL: Video(prefix + "hello_hello.mp4"),
            TvShowState.PRESS_5: Video(prefix + "press_5.mp4"),
            TvShowState.GOING_SCYLLA: Video(prefix + "scylla_cave.mp4"),
            TvShowState.ENDING: Video(prefix + "you_lost.mp4"),
            TvShowState.HAVE_LUCK: Video(prefix + "have_luck.mp4"),
        }

        self.state = TvShowState.ATTRACT
        self.state_start = time.time()
        self.current_game = next(iter(games))

        self.cave = None
        self.forest = None

        self.pre_cave_score = 0


    def handle_events(self, phone_events: PhoneEvents):
        if self.state != TvShowState.ATTRACT and phone_events.hungup:
                self.switch_to(TvShowState.ATTRACT)
                if self.cave is not None:
                    self.cave.end()
                    self.cave = None
                elif self.forest is not None:
                    self.forest.end()
                    self.forest = None

        if self.state == TvShowState.ATTRACT:
            self.reloop()

            if phone_events.offhook:
                 self.switch_to(TvShowState.INITIAL)

            if phone_events.press_5:
                 self.switch_to(TvShowState.PLAYING_HUGO)

        elif self.state == TvShowState.INITIAL:
            if self.has_ended():
                self.switch_to(TvShowState.PRESS_5)

        elif self.state == TvShowState.PRESS_5:
            self.reloop()

            if phone_events.press_5:
                self.switch_to(TvShowState.HAVE_LUCK)

        elif self.state == TvShowState.HAVE_LUCK:
            if self.has_ended():
                self.switch_to(TvShowState.INSTRUCTIONS)
                self.set_random_game()

        elif self.state == TvShowState.INSTRUCTIONS:
            if phone_events.press_5 or self.state_timeout(self.INSTRUCTIONS_TIMEOUT):
                self.switch_to(TvShowState.PLAYING_HUGO)

        elif self.state == TvShowState.PLAYING_HUGO:
            if self.forest is None:
                self.forest = Forest()

            self.forest.process_events(phone_events)

            if self.forest.ended:
                self.pre_cave_score = self.forest.score
                self.forest = None
                self.switch_to(TvShowState.GOING_SCYLLA)

        elif self.state == TvShowState.GOING_SCYLLA:
            if self.has_ended():
                self.switch_to(TvShowState.CAVE)

        elif self.state == TvShowState.CAVE:
            if self.cave is None:
                self.cave = Cave(self.pre_cave_score)

            self.cave.process_events(phone_events)

            if self.cave.ended:
                self.cave = None
                self.switch_to(TvShowState.ENDING)

    def render(self, display, instructions):
        if self.state == TvShowState.CAVE:
             if self.cave is not None:
                 self.cave.render(display)
        elif self.state == TvShowState.PLAYING_HUGO:
             if self.forest is not None:
                 self.forest.render(display)
        elif self.state == TvShowState.INSTRUCTIONS:
             display.blit(instructions[self.current_game], (0,0))
        else:
            vid_draw = self.videos[self.state] if self.state in self.videos else None

            if vid_draw:
                vid_draw.draw(display, (0, 0))

    def is_playing(self):
        return self.state != TvShowState.ATTRACT

