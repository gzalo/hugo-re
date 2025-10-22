import math
import random

import pygame

import global_state
from audio_helper import AudioHelper
from config import Config
from effect_type import EffectType
from forest.forest_resources import ForestResources
from forest.hurt_branch_animation import HurtBranchAnimation
from forest.hurt_flying_start import HurtFlyingStart
from forest.hurt_rock_animation import HurtRockAnimation
from forest.hurt_trap_animation import HurtTrapAnimation
from forest.win_talking import WinTalking
from game_data import GameData
from phone_events import PhoneEvents
from render_type import RenderType
from state import State


class Playing(State):
    def __init__(self, context: GameData):
        super().__init__(context)

        self.arrow_up_focus = False
        self.arrow_down_focus = False
        self.hugo_jumping_time = None
        self.hugo_crawling_time = None
        self.last_time = global_state.frame_time
        self.old_second = None
        self.HUGO_X_POS = 16

        self.needs_bottom = RenderType.PRE
        self.needs_background = RenderType.PRE

    def process_events(self, phone_events: PhoneEvents):
        if not self.arrow_up_focus and not self.arrow_down_focus:
            if phone_events.press_2:
                self.arrow_up_focus = True
                self.hugo_jumping_time = global_state.frame_time

            if phone_events.press_8:
                self.arrow_down_focus = True
                self.hugo_crawling_time = global_state.frame_time

        if self.context.forest_parallax_pos > Config.FOREST_MAX_TIME:
            self.context.forest_parallax_pos = Config.FOREST_MAX_TIME
            return WinTalking
        else:
            self.context.forest_parallax_pos += global_state.frame_time - self.last_time
            self.last_time = global_state.frame_time

        _, integer = math.modf(self.context.forest_parallax_pos)
        integer = math.floor(integer) + 1

        if integer >= Config.FOREST_MAX_TIME:
            integer = Config.FOREST_MAX_TIME - 1

        if self.old_second is None:
            self.old_second = math.floor(self.context.forest_parallax_pos)

        if self.arrow_up_focus and global_state.frame_time - self.hugo_jumping_time > 0.75:
            self.hugo_jumping_time = None
            self.arrow_up_focus = False
        if self.arrow_down_focus and global_state.frame_time - self.hugo_crawling_time > 0.75:
            self.hugo_crawling_time = None
            self.arrow_down_focus = False

        if self.old_second != math.floor(self.context.forest_parallax_pos):

            if self.context.forest_obstacles[integer] != 0 and not Config.GOD_MODE:
                if self.context.forest_obstacles[integer] == 1 and not self.arrow_up_focus:  # Catapult
                    AudioHelper.play(ForestResources.sfx_hugo_launch, self.context.audio_port)
                    AudioHelper.play(ForestResources.sfx_catapult_eject, self.context.audio_port)
                    self.context.forest_obstacles[integer] = 0
                    return HurtFlyingStart
                elif self.context.forest_obstacles[integer] == 2 and not self.arrow_up_focus:  # Trap
                    AudioHelper.play(ForestResources.sfx_hugo_hittrap, self.context.audio_port)
                    self.context.forest_obstacles[integer] = 0
                    return HurtTrapAnimation
                elif self.context.forest_obstacles[integer] == 3 and not self.arrow_up_focus:  # Rock
                    AudioHelper.play(ForestResources.sfx_hugo_hitlog, self.context.audio_port)
                    self.context.forest_obstacles[integer] = 0
                    return HurtRockAnimation
                elif self.context.forest_obstacles[integer] == 4:  # Tree
                    if self.arrow_down_focus:
                        AudioHelper.play(ForestResources.sfx_tree_swush, self.context.audio_port)
                    else:
                        AudioHelper.play(ForestResources.sfx_hugo_hitlog, self.context.audio_port)
                        self.context.forest_obstacles[integer] = 0
                        return HurtBranchAnimation

            if self.arrow_up_focus and self.context.forest_sacks[integer] != 0:
                if self.context.forest_sacks[integer] == 1:
                    self.context.forest_score += 100
                    AudioHelper.play(ForestResources.sfx_sack_normal, self.context.audio_port)
                    self.context.forest_sacks[integer] = 0
                    global_state.attacks.append((self.context.country, EffectType.SPLAT, global_state.frame_time))
                elif self.context.forest_sacks[integer] == 2:
                    self.context.forest_score += 250
                    AudioHelper.play(ForestResources.sfx_sack_bonus, self.context.audio_port)
                    self.context.forest_sacks[integer] = 0
                    global_state.attacks.append((self.context.country, EffectType.INVERT, global_state.frame_time))

        if self.get_frame_index() % 8 == 0 and not self.arrow_up_focus:
            walk_sfx = [ForestResources.sfx_hugo_walk0, ForestResources.sfx_hugo_walk1, ForestResources.sfx_hugo_walk2,
                        ForestResources.sfx_hugo_walk3, ForestResources.sfx_hugo_walk4]
            AudioHelper.play(walk_sfx[random.randint(0, 4)], self.context.audio_port)

        self.old_second = math.floor(self.context.forest_parallax_pos)

        return None

    def render(self, screen):
        fract, _ = math.modf(self.context.forest_parallax_pos)

        for index in range(len(self.context.forest_obstacles)):
            obstacle_pos = (index - self.context.forest_parallax_pos) * Config.FOREST_GROUND_SPEED

            if self.context.forest_obstacles[index] == 1:  # Catapult
                idx = self.get_frame_index() % len(ForestResources.catapult)
                dy = [45, 43, 39, 34, 29, 22, 14, 1]
                screen.blit(ForestResources.catapult[idx], (obstacle_pos - 8, 112 + dy[idx]))
            if self.context.forest_obstacles[index] == 2:  # Trap
                idx = self.get_frame_index() % len(ForestResources.trap)
                dy = [176, 173, 169, 165, 176, 176]
                screen.blit(ForestResources.trap[idx], (obstacle_pos - 8, dy[idx] - 24))
            if self.context.forest_obstacles[index] == 3:  # Rock
                idx = self.get_frame_index() % len(ForestResources.rock)
                screen.blit(ForestResources.rock[idx], (obstacle_pos - math.sin(fract * (2 * math.pi)) * 15, 120))
            if self.context.forest_obstacles[index] == 4:  # Tree
                idx = self.get_frame_index() % len(ForestResources.tree)
                screen.blit(ForestResources.lone_tree[0], (obstacle_pos - 52, -40))
                screen.blit(ForestResources.tree[idx], (obstacle_pos, 52 + 10))

        for index in range(len(self.context.forest_sacks)):
            sack_pos = (index - self.context.forest_parallax_pos) * Config.FOREST_GROUND_SPEED

            if self.context.forest_sacks[index] != 0:
                screen.blit(ForestResources.sack[0], (sack_pos-16, 32))

        for index in range(len(self.context.forest_leaves)):
            leave_pos = (index - self.context.forest_parallax_pos) * Config.FOREST_GROUND_SPEED
            if self.context.forest_leaves[index] == 1:
                screen.blit(ForestResources.leaves2[0], (leave_pos-16, -10))
            elif self.context.forest_leaves[index] == 2:
                screen.blit(ForestResources.leaves1[0], (leave_pos-16, -10))

        if self.arrow_up_focus:
            screen.blit(ForestResources.arrows[1], (256, 17))
        else:
            screen.blit(ForestResources.arrows[0], (256 + 2, 16 + 3))
        if self.arrow_down_focus:
            screen.blit(ForestResources.arrows[3], (256, 54))
        else:
            screen.blit(ForestResources.arrows[2], (256 + 2, 54 + 2))

        if self.arrow_up_focus:
            dt = (global_state.frame_time - self.hugo_jumping_time) / 0.75
            dy = -250 * dt ** 2 + 250 * dt - 22.5
            screen.blit(ForestResources.hugo_jump[self.get_frame_index() % len(ForestResources.hugo_jump)], (self.HUGO_X_POS, 40 - dy))
        elif self.arrow_down_focus:
            screen.blit(ForestResources.hugo_crawl[self.get_frame_index() % len(ForestResources.hugo_crawl)], (self.HUGO_X_POS, 105))
        else:
            screen.blit(ForestResources.hugo_side[self.get_frame_index() % len(ForestResources.hugo_side)], (self.HUGO_X_POS, 90))

