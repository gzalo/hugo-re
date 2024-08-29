import pygame

from config import Config


class PostProcessing:
    def __init__(self):
        self.wavyness = 0
        self.bitcrush = 256
        self.hue_shift = 0
        self.scale = 1
        pygame.joystick.init()
        self.joysticks = [pygame.joystick.Joystick(x) for x in range(pygame.joystick.get_count())]

    def handle_events(self):
        if len(self.joysticks) == 0:
            return

        if self.joysticks[0].get_button(0):
            self.wavyness -= 0.01
        if self.joysticks[0].get_button(1):
            self.wavyness += 0.01

        if self.joysticks[0].get_button(2):
            Config.FOREST_BG_SPEED_MULTIPLIER -= 0.1
        if self.joysticks[0].get_button(3):
            Config.FOREST_BG_SPEED_MULTIPLIER += 0.1

        if self.joysticks[0].get_button(4):
            self.bitcrush *= 2
        if self.joysticks[0].get_button(5):
            self.bitcrush /= 2

        if self.joysticks[0].get_button(6):
            self.hue_shift += 0.01
        if self.joysticks[0].get_button(7):
            self.hue_shift -= 0.01

        if self.joysticks[0].get_button(8):
            self.scale += 0.1
        if self.joysticks[0].get_button(9):
            self.scale -= 0.1

        self.bitcrush = max(4, min(256, self.bitcrush))
        self.wavyness = max(0, min(0.1, self.wavyness))
        self.scale = max(1, min(2, self.scale))

        if self.hue_shift < 0:
            self.hue_shift += 1
        if self.hue_shift > 1:
            self.hue_shift -= 1

        Config.FOREST_BG_SPEED_MULTIPLIER = max(1, min(5, Config.FOREST_BG_SPEED_MULTIPLIER))

    def apply(self, program, any_playing):
        if any_playing:
            program['wavyness'] = self.wavyness
            program['bitcrush'] = self.bitcrush
            program['hue_shift'] = self.hue_shift
            program['scale'] = self.scale
        else:
            program['wavyness'] = 0.01
            program['bitcrush'] = 256
            program['hue_shift'] = 0
            program['scale'] = 1
