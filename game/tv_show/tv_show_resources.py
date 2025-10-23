import pygame
from pyvidplayer2 import Video

from config import Config


class TvShowResources:
    instructions = None
    videos_attract = {}
    videos_initial = {}
    videos_press_5 = {}
    videos_going_scylla = {}
    videos_ending = {}
    videos_have_luck = {}
    audio_attract = {}
    audio_initial = {}
    audio_press_5 = {}
    audio_going_scylla = {}
    audio_ending = {}
    audio_have_luck = {}
    press_5 = None

    @staticmethod
    def init():
        TvShowResources.press_5 = pygame.image.load("resources/images/press_5.png").convert_alpha()
        TvShowResources.instructions = {game_name: pygame.image.load("resources/images/instruction_" + game_name + ".png").convert() for
                    game_name in Config.GAMES.keys()}

        for country in Config.COUNTRIES:
            prefix = f"resources/videos/{country}/"
            audio_prefix = f"audio_for_videos/{country}/"
            
            TvShowResources.videos_attract[country] = Video(prefix + "attract_demo.avi", no_audio=True)
            TvShowResources.videos_initial[country] = Video(prefix + "hello_hello.avi", no_audio=True)
            TvShowResources.videos_press_5[country] = Video(prefix + "press_5.avi", no_audio=True)
            TvShowResources.videos_going_scylla[country] = Video(prefix + "scylla_cave.avi", no_audio=True)
            TvShowResources.videos_ending[country] = Video(prefix + "you_lost.avi", no_audio=True)
            TvShowResources.videos_have_luck[country] = Video(prefix + "have_luck.avi", no_audio=True)
            
            TvShowResources.audio_attract[country] = audio_prefix + "attract_demo.aac"
            TvShowResources.audio_initial[country] = audio_prefix + "hello_hello.aac"
            TvShowResources.audio_press_5[country] = audio_prefix + "press_5.aac"
            TvShowResources.audio_going_scylla[country] = audio_prefix + "scylla_cave.aac"
            TvShowResources.audio_ending[country] = audio_prefix + "you_lost.aac"
            TvShowResources.audio_have_luck[country] = audio_prefix + "have_luck.aac"
