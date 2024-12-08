import random

import pygame

from config import Config
from tween import Tween


class Splat:
    splats = None
    orb = None

    @staticmethod
    def init():
        Splat.splats = [pygame.image.load("resources/splats/splat0" + str(idx) + ".png").convert_alpha() for idx in range(10)]
        Splat.orb = pygame.image.load("resources/orbs/orb.png").convert_alpha()

    @staticmethod
    def render_splat(screen, time):
        surface = Splat.splats[random.randint(0, 9)]

        if time > Config.EFFECT_DURATION_SPLAT / 2:
            alpha = int(Tween.map(time, Config.EFFECT_DURATION_SPLAT / 2, Config.EFFECT_DURATION_SPLAT, 255, 0))
            surface.set_alpha(alpha)
        else:
            surface.set_alpha(255)

        screen.blit(surface, (0, 0))
