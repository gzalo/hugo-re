import math
import random
import time

import pygame

from config import Config
from forest.forest_resources import ForestResources
from forest.hurt_branch_animation import HurtBranchAnimation
from forest.hurt_flying_start import HurtFlyingStart
from forest.hurt_rock_animation import HurtRockAnimation
from forest.hurt_trap_animation import HurtTrapAnimation
from forest.win_talking import WinTalking
from phone_events import PhoneEvents
from state import State


class Playing(State):
    def __init__(self, parent):
        super().__init__(parent)
        self.arrow_up_focus = False
        self.arrow_down_focus = False
        self.hugo_jumping = False
        self.hugo_crawling = False
        self.last_time = time.time()
        self.old_second = None

    def process_events(self, phone_events: PhoneEvents):
        if phone_events.press_2 and not self.arrow_down_focus:
            self.arrow_up_focus = True

        if phone_events.press_8 and not self.arrow_up_focus:
            self.arrow_down_focus = True

        if self.parent.parallax_pos > Config.FOREST_MAX_TIME:
            self.parent.parallax_pos = Config.FOREST_MAX_TIME
            return WinTalking(self.parent)
        else:
            self.parent.parallax_pos += time.time() - self.last_time
            self.last_time = time.time()

        _, integer = math.modf(self.parent.parallax_pos)
        integer = math.floor(integer) + 1

        if integer >= Config.FOREST_MAX_TIME:
            integer = Config.FOREST_MAX_TIME - 1

        if self.old_second is None:
            self.old_second = math.floor(self.parent.parallax_pos)

        if self.old_second != math.floor(self.parent.parallax_pos):
            if self.hugo_jumping:
                self.hugo_jumping = False
                self.arrow_up_focus = False
            if self.hugo_crawling:
                self.hugo_crawling = False
                self.arrow_down_focus = False

            if self.arrow_up_focus:
                self.hugo_jumping = True
            elif self.arrow_down_focus:
                self.hugo_crawling = True

            if self.parent.obstacles[integer] != 0:
                if self.parent.obstacles[integer] == 1 and not self.hugo_jumping:  # Catapult
                    pygame.mixer.Sound.play(ForestResources.speak_catapult_hit)
                    pygame.mixer.Sound.play(ForestResources.sfx_hugo_launch)
                    pygame.mixer.Sound.play(ForestResources.sfx_catapult_eject)
                    self.parent.obstacles[integer] = 0
                    return HurtFlyingStart(self.parent)
                elif self.parent.obstacles[integer] == 2 and not self.hugo_jumping:  # Trap
                    pygame.mixer.Sound.play(ForestResources.sfx_hugo_hittrap)
                    self.parent.obstacles[integer] = 0
                    return HurtTrapAnimation(self.parent)
                elif self.parent.obstacles[integer] == 3 and not self.hugo_jumping:  # Rock
                    pygame.mixer.Sound.play(ForestResources.sfx_hugo_hitlog)
                    self.parent.obstacles[integer] = 0
                    return HurtRockAnimation(self.parent)
                elif self.parent.obstacles[integer] == 4:  # Tree
                    if self.hugo_crawling:
                        pygame.mixer.Sound.play(ForestResources.sfx_tree_swush)
                    else:
                        pygame.mixer.Sound.play(ForestResources.sfx_hugo_hitlog)
                        self.parent.obstacles[integer] = 0
                        return HurtBranchAnimation(self.parent)

            if self.hugo_jumping and self.parent.sacks[integer] != 0:
                if self.parent.sacks[integer] == 1:
                    self.parent.score += 100
                    pygame.mixer.Sound.play(ForestResources.sfx_sack_normal)
                    self.parent.sacks[integer] = 0
                elif self.parent.sacks[integer] == 2:
                    self.parent.score += 250
                    pygame.mixer.Sound.play(ForestResources.sfx_sack_bonus)
                    self.parent.sacks[integer] = 0

        if self.get_frame_index() % 8 == 0 and not self.hugo_jumping:
            walk_sfx = [ForestResources.sfx_hugo_walk0, ForestResources.sfx_hugo_walk1, ForestResources.sfx_hugo_walk2,
                        ForestResources.sfx_hugo_walk3, ForestResources.sfx_hugo_walk4]
            pygame.mixer.Sound.play(walk_sfx[random.randint(0, 4)])

        self.old_second = math.floor(self.parent.parallax_pos)

    def render(self, screen):
        self.parent.render_background(screen)
        self.parent.render_bottom(screen)
        fract, _ = math.modf(self.parent.parallax_pos)

        for index in range(len(self.parent.obstacles)):
            obstacle_pos = (index - self.parent.parallax_pos) * Config.FOREST_GROUND_SPEED

            if self.parent.obstacles[index] == 1:  # Catapult
                idx = self.get_frame_index() % len(ForestResources.catapult)
                dy = [45, 43, 39, 34, 29, 22, 14, 1]
                screen.blit(ForestResources.catapult[idx], (obstacle_pos - 8, 112 + dy[idx]))
            if self.parent.obstacles[index] == 2:  # Trap
                idx = self.get_frame_index() % len(ForestResources.trap)
                dy = [176, 173, 169, 165, 176, 176]
                screen.blit(ForestResources.trap[idx], (obstacle_pos - 8, dy[idx] - 24))
            if self.parent.obstacles[index] == 3:  # Rock
                idx = self.get_frame_index() % len(ForestResources.rock)
                screen.blit(ForestResources.rock[idx], (obstacle_pos - math.sin(fract * (2 * math.pi)) * 15, 120))
            if self.parent.obstacles[index] == 4:  # Tree
                idx = self.get_frame_index() % len(ForestResources.tree)
                screen.blit(ForestResources.lone_tree[0], (obstacle_pos - 52, -40))
                screen.blit(ForestResources.tree[idx], (obstacle_pos, 52 + 10))

        for index in range(len(self.parent.sacks)):
            sack_pos = (index - self.parent.parallax_pos) * Config.FOREST_GROUND_SPEED

            if self.parent.sacks[index] != 0:
                screen.blit(ForestResources.sack[0], (sack_pos, 32))

        for index in range(len(self.parent.leaves)):
            leave_pos = (index - self.parent.parallax_pos) * Config.FOREST_GROUND_SPEED
            if self.parent.leaves[index] == 1:
                screen.blit(ForestResources.leaves2[0], (leave_pos, -10))
            elif self.parent.leaves[index] == 2:
                screen.blit(ForestResources.leaves1[0], (leave_pos, -10))

        if self.arrow_up_focus:
            screen.blit(ForestResources.arrows[1], (256, 17))
        else:
            screen.blit(ForestResources.arrows[0], (256 + 2, 16 + 3))
        if self.arrow_down_focus:
            screen.blit(ForestResources.arrows[3], (256, 54))
        else:
            screen.blit(ForestResources.arrows[2], (256 + 2, 54 + 2))

        if self.hugo_jumping:
            dy = -250 * fract ** 2 + 250 * fract - 22.5
            screen.blit(ForestResources.hugo_jump[self.get_frame_index() % len(ForestResources.hugo_jump)], (0, 40 - dy))
        elif self.hugo_crawling:
            screen.blit(ForestResources.hugo_crawl[self.get_frame_index() % len(ForestResources.hugo_crawl)], (0, 105))
        else:
            screen.blit(ForestResources.hugo_side[self.get_frame_index() % len(ForestResources.hugo_side)], (0, 90))