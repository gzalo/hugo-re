import pygame

from config import Config
from tween import Tween


class PostProcessing:
    def __init__(self):
        self.wavyness = 0
        self.bitcrush = 256
        self.hue_shift = 0
        self.scale = 1
        self.line_glitch = 0
        self.chromatic_aberration = 0
        pygame.joystick.init()
        self.joysticks = [pygame.joystick.Joystick(x) for x in range(pygame.joystick.get_count())]

    def handle_events(self):
        if len(self.joysticks) == 0:
            return

        if self.joysticks[0].get_numaxes() != 6:
            return

        self.scale = Tween.map(self.joysticks[0].get_axis(0), -1, 1, 1, 4)
        Config.FOREST_BG_SPEED_MULTIPLIER = Tween.map(self.joysticks[0].get_axis(1), -1, 1, 1, 5)
        self.wavyness = Tween.map(self.joysticks[0].get_axis(2), -1, 1, 0, 0.1)
        self.hue_shift = Tween.map(self.joysticks[0].get_axis(3), -1, 1, 0, 1)
        self.bitcrush = Tween.map_bitcrush(self.joysticks[0].get_axis(4), -1, 1, 2, 256)
        self.line_glitch = Tween.map(self.joysticks[0].get_axis(4), -1, 1, 0, 0.5)
        self.chromatic_aberration = Tween.map(self.joysticks[0].get_axis(5), -1, 1, 0, 1)

    def apply(self, program, any_playing):
        if any_playing:
            program['wavyness'] = self.wavyness
            program['bitcrush'] = self.bitcrush
            program['hue_shift'] = self.hue_shift
            program['scale'] = self.scale
            program['chromatic_aberration'] = self.chromatic_aberration
            program['line_glitch'] = self.line_glitch
        else:
            program['wavyness'] = 0.01
            program['bitcrush'] = 256
            program['hue_shift'] = 0
            program['scale'] = 1
            program['chromatic_aberration'] = 0
            program['line_glitch'] = 0

