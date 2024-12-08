import time

import pygame
import random

from config import Config
from effect_type import EffectType
from effects.splat import Splat
from forest.forest_resources import ForestResources
from forest.playing import Playing
from forest.talking_after_hurt import TalkingAfterHurt
from forest.talking_game_over import TalkingGameOver
from forest.wait_intro import WaitIntro
from null_state import NullState
from phone_events import PhoneEvents


class ForestGame:

    def __init__(self, parent):
        self.parent = parent

        self.score = 0
        self.ended = False
        self.lives = 3

        self._state = WaitIntro(self)
        self._state.on_enter()

        self.parallax_pos = 0

        self.obstacles = self.generate_obstacles()
        self.sacks = self.generate_sacks()
        self.leaves = self.generate_leaves()
        self.effect_status = None
        self.effect_start = None

        pygame.mixer.Sound.play(ForestResources.sfx_bg_atmosphere, loops=-1)

    def process_events(self, phone_events: PhoneEvents):
        if self.effect_status == EffectType.INVERT and time.time() - self.effect_start > Config.EFFECT_DURATION_ORB:
            [phone_events.press_2, phone_events.press_8] = [phone_events.press_8, phone_events.press_2]

        next_state = self._state.process_events(phone_events)

        if self.effect_status == EffectType.SPLAT and time.time() - self.effect_start > Config.EFFECT_DURATION_SPLAT:
            self.effect_status = None
        if self.effect_status == EffectType.INVERT and time.time() - self.effect_start > Config.EFFECT_DURATION_INVERT:
            self.effect_status = None

        if next_state is not None:
            self._state.on_exit()
            self._state = next_state
            self._state.on_enter()
            if isinstance(next_state, NullState):
                self.end()

    def render(self, screen):
        self._state.render(screen)

        if self.effect_start:
            dt = time.time() - self.effect_start

            if dt < Config.EFFECT_DURATION_ORB:
                return

            if self.effect_status == EffectType.SPLAT:
                Splat.render_splat(screen, time.time() - self.effect_start)

            if self.effect_status == EffectType.INVERT and isinstance(self._state, Playing):
                screen.blit(ForestResources.inverted_arrows, (0, 0))

    def reduce_lives(self):
        self.lives -= 1
        if self.lives == 0:
            return TalkingGameOver(self)
        else:
            return TalkingAfterHurt(self)

    def external_effect(self, effect):
        if self.effect_status is not None:
            return
        self.effect_status = effect
        self.effect_start = time.time()

    def render_background(self, screen):
        hills_speed = 6 * Config.FOREST_BG_SPEED_MULTIPLIER
        trees_speed = 12 * Config.FOREST_BG_SPEED_MULTIPLIER
        grass_speed = 30 * Config.FOREST_BG_SPEED_MULTIPLIER
        hills_width = ForestResources.bg_hillsday[0].get_width()
        trees_width = ForestResources.bg_trees[0].get_width()
        grass_width = ForestResources.grass[0].get_width()
        ground_width = ForestResources.bg_ground[0].get_width()

        hills_pos = (self.new_mod(- self.parallax_pos * hills_speed, hills_width), 0)
        trees_pos = (self.new_mod(- self.parallax_pos * trees_speed, trees_width), -24)
        grass_pos = (self.new_mod(- self.parallax_pos * grass_speed, grass_width), 172)
        ground_pos = (self.new_mod(- self.parallax_pos * Config.FOREST_GROUND_SPEED, ground_width), 158)
        hills_pos_next = (hills_pos[0] + hills_width, hills_pos[1])
        tress_pos_next = (trees_pos[0] + trees_width, trees_pos[1])

        mountain_pos = 320 - 96 - (self.parallax_pos - Config.FOREST_MAX_TIME) * Config.FOREST_GROUND_SPEED

        screen.blit(ForestResources.bg_gradient, (0,0))
        screen.blit(ForestResources.bg_hillsday[0], hills_pos)
        screen.blit(ForestResources.bg_hillsday[0], hills_pos_next)

        screen.blit(ForestResources.bg_trees[0], trees_pos)
        screen.blit(ForestResources.bg_trees[0], tress_pos_next)

        for i in range(5):
            ground_x = ground_pos[0] + i * ground_width
            screen.blit(ForestResources.bg_ground[0], (ground_x, ground_pos[1]))

        for i in range(12):
            grass_x = grass_pos[0] + i * grass_width
            screen.blit(ForestResources.grass[0], (grass_x, grass_pos[1]))

        screen.blit(ForestResources.end_mountain[0], (mountain_pos, -16))

    def render_bottom(self, screen):
        screen.blit(ForestResources.scoreboard, (0, 184))
        for i in range(self.lives):
            screen.blit(ForestResources.hugo_lives[0], (i * 40 + 32, 188))

        x_score = 200
        y_score = 194
        x_space = 24
        thousands = self.score // 1000
        hundreds = (self.score - thousands * 1000) // 100
        tens = (self.score - thousands * 1000 - hundreds * 100) // 10
        ones = self.score - thousands * 1000 - hundreds * 100 - tens * 10

        screen.blit(ForestResources.score_numbers[0], (x_score + x_space * 0, y_score), self.get_spritesheet_area(thousands))
        screen.blit(ForestResources.score_numbers[0], (x_score + x_space * 1, y_score), self.get_spritesheet_area(hundreds))
        screen.blit(ForestResources.score_numbers[0], (x_score + x_space * 2, y_score), self.get_spritesheet_area(tens))
        screen.blit(ForestResources.score_numbers[0], (x_score + x_space * 3, y_score), self.get_spritesheet_area(ones))

    def end(self):
        self._state.on_exit()
        pygame.mixer.Sound.stop(ForestResources.sfx_bg_atmosphere)
        self.ended = True

    @staticmethod
    def get_spritesheet_area(value):
        width = 32
        height = 33

        xpos = value % 5
        ypos = value // 5

        return 1 + xpos * (width + 1), 1 + ypos * (height + 1), width, height

    @staticmethod
    def new_mod(a, b):
        res = a % b
        return res if not res else res - b if a < 0 else res

    @staticmethod
    def generate_obstacles():
        empty_prob = 0.65
        other_prob = (1 - empty_prob) / 4
        out = random.choices([0, 1, 2, 3, 4], weights=[empty_prob, other_prob, other_prob, other_prob, other_prob],
                             k=Config.FOREST_MAX_TIME)
        out[0] = 0
        out[1] = 0
        out[2] = 0
        out[3] = 0
        for idx in range(len(out) - 1):
            if out[idx] != 0:
                out[idx + 1] = 0

        return out

    @staticmethod
    def generate_sacks():
        empty_prob = 0.7
        other_prob = (1 - empty_prob) * 0.7
        other_prob_high = (1 - empty_prob) * 0.3
        return random.choices([0, 1, 2], weights=[empty_prob, other_prob, other_prob_high], k=Config.FOREST_MAX_TIME)

    @staticmethod
    def generate_leaves():
        arr = random.choices([1, 2], weights=[0.5, 0.5], k=Config.FOREST_MAX_TIME)
        for idx in range(len(arr) - 1):
            if arr[idx] == 2:
                arr[idx + 1] = 0
        return arr

