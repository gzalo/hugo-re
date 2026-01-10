import pygame

from resource import Resource


class ScoreboardResources:
    background = None

    hugo_side = None

    icon_sack = None
    icon_finish = None
    icon_golden_sack = None

    icon_obstacle_catapult = None
    icon_obstacle_trap = None
    icon_obstacle_rock = None
    icon_obstacle_branch = None

    score_font = None

    score_counter = None

    @staticmethod
    def init():
        ScoreboardResources.background = Resource.load_surface_raw("ScoreboardData", "background.png")

        ScoreboardResources.hugo_side = Resource.load_surfaces("ScoreboardData", "hugo_side.cgf", 0, 15)

        ScoreboardResources.icon_sack = Resource.load_surface_raw("ScoreboardData", "icon_sack.png")
        ScoreboardResources.icon_finish = Resource.load_surface_raw("ScoreboardData", "icon_finish.png")
        ScoreboardResources.icon_golden_sack = Resource.load_surface_raw("ScoreboardData", "icon_golden_sack.png")

        ScoreboardResources.icon_obstacle_catapult = Resource.load_surface_raw("ScoreboardData", "icon_obstacle_catapult.png")
        ScoreboardResources.icon_obstacle_trap = Resource.load_surface_raw("ScoreboardData", "icon_obstacle_trap.png")
        ScoreboardResources.icon_obstacle_rock = Resource.load_surface_raw("ScoreboardData", "icon_obstacle_rock.png")
        ScoreboardResources.icon_obstacle_branch = Resource.load_surface_raw("ScoreboardData", "icon_obstacle_branch.png")

        ScoreboardResources.score_font = Resource.load_surfaces("RopeOutroData", "SCORE.cgf", 0, 9)

        ScoreboardResources.score_counter = Resource.load_sfx("RopeOutroData", "COUNTER.WAV")
