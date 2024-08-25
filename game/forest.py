from enum import Enum
from resource import Resource
import time
import pygame
import random
import math

class ForestState(Enum):
    WAIT_INTRO = 0
    PLAYING = 1
    SCYLLA_BUTTON = 2
    HURT_TRAP_ANIMATION = 3
    HURT_TRAP_TALKING = 4
    HURT_FLYING_START = 5
    HURT_FLYING_TALKING = 6
    HURT_FLYING_FALLING = 7
    HURT_FLYING_FALLING_HANG_ANIMATION = 8
    HURT_FLYING_FALLING_HANG_TALKING = 9
    HURT_ROCK_ANIMATION = 10
    HURT_ROCK_HIT_ANIMATION = 11
    HURT_ROCK_TALKING = 12
    HURT_BRANCH_ANIMATION = 13
    HURT_BRANCH_TALKING = 14
    TALKING_AFTER_HURT = 15
    TALKING_GAME_OVER = 16
    WIN_DIALOGUE = 17

class Forest:
    state = None
    state_start = None

    def __init__(self):
        self.score = 0
        self.ended = False
        self.lifes = 3
        self.played = {}
        self.background_music_playing = False
        self.parallax_pos = 0
        self.scylla_affected = False
        self.arrow_up_focus = False
        self.arrow_down_focus = False
        self.last_time = None
        self.end_max_time = 60
        self.ground_speed = 64
        self.old_second = None
        self.hugo_jumping = False
        self.hugo_crawling = False

        self.obstacles = self.generate_obstacles()
        self.sacks = self.generate_sacks()
        self.leaves = self.generate_leaves()

        self.bg_hillsday = Resource.load_surfaces("ForestData", "hillsday.cgf", 0,0) # hills night
        # self.bg_hillsnight = Resource.load_surfaces("ForestData", "hillsngt.cgf" ,0,0) # hills day
        self.bg_trees = Resource.load_surfaces("ForestData", "paratrees.cgf" ,0,0) # more background
        self.bg_spooky_trees = Resource.load_surfaces("ForestData", "spooky.cgf", 0,0) # far background
        self.bg_ground = Resource.load_surfaces("ForestData", "paraground.cgf",0,0) # ground
        self.grass = Resource.load_surfaces("ForestData", "grass.cgf", 0,0) # near glass
        self.leaves1 = Resource.load_surfaces("ForestData", "leaves1.cgf",0,0)  # bigger leaves
        self.leaves2 = Resource.load_surfaces("ForestData", "leaves2.cgf",0,0) # upper leaves
        self.scoreboard = Resource.load_surface_raw("ForestData", "scorebrd.bmp") # lower portion
        self.bg_gradient = Resource.load_surface_raw("ForestData", "gradient.bmp")  # background gradient
        self.end_mountain = Resource.load_surfaces("ForestData", "wall.cgf",0,0) # ending wall
        # self.hugo_flyv = Resource.load_surfaces("ForestData", "hugoflyv.cgf", 0,0) # looking up preocupated, hit by branch
        self.hugo_side = Resource.load_surfaces("ForestData", "hugoside.cgf", 0, 7) # hugo walking
        self.hugo_crawl = Resource.load_surfaces("ForestData", "kravle.cgf", 0, 7) # crawls
        self.hugo_jump = Resource.load_surfaces("ForestData", "hugohop.cgf", 0,2) # hugo jumps
        # self.hugo_scared = Resource.load_surfaces("ForestData", "scared.cgf", 0,7) # scared, when?
        # self.hugo_end = Resource.load_surfaces("ForestData", "hugoend.cgf", 0,0) # looks at camera after ending
        # self.hugo_intrap = Resource.load_surfaces("ForestData", "caught.cgf",0,0) # in trap
        self.hugo_telllives = Resource.load_surfaces("ForestData", "hugo_hello.cgf",0,15) # hugo tells remaining lives
        self.hugo_hand1 = Resource.load_surfaces("ForestData", "hand1.cgf",0,0) # hand far
        self.hugo_hand2 = Resource.load_surfaces("ForestData", "hand2.cgf",0,0) # hand closer
        self.tree = Resource.load_surfaces("ForestData", "branch-swing.cgf",0,6) # branch swinging
        self.lone_tree = Resource.load_surfaces("ForestData", "lonetree.cgf", 0,0) # single tree
        self.rock = Resource.load_surfaces("ForestData", "stone.cgf",0,7) # rotating boulder
        self.sack = Resource.load_surfaces("ForestData", "saek.cgf", 0, 3) # bags
        self.trap = Resource.load_surfaces("ForestData", "faelde.cgf", 0, 5) # trap in floor
        self.catapult = Resource.load_surfaces("ForestData", "catapult.cgf", 0, 7) # catapult in floor
        self.hit_rock = Resource.load_surfaces("ForestData", "hgrock.til", 0, 60) # hit by ball
        self.hugo_lookrock = Resource.load_surfaces("ForestData", "hugo-rock.til", 0, 14) # almost crushed
        self.hit_rock_sync = Resource.load_surfaces("ForestData", "msyncrck.til", 0, 17) # talking after hit by ball
        self.catapult_fly = Resource.load_surfaces("ForestData", "hgkatfly.til", 0, 113) # flying in trap
        self.catapult_fall = Resource.load_surfaces("ForestData", "hgkatfly.til", 115, 189)  # falling in trap
        self.catapult_airtalk = Resource.load_surfaces("ForestData", "catapult-speak.til", 0, 15)  # talking in air
        self.catapult_hang = Resource.load_surfaces("ForestData", "hgkathng.til", 0, 12) # hanging from branch
        self.catapult_hangspeak = Resource.load_surfaces("ForestData", "hanging_mouth.cgf",0,11) # mouth parts when hanging
        self.hugohitlog = Resource.load_surfaces("ForestData", "branch-groggy.til",0,42) # hurt with branch
        self.hugohitlog_talk = Resource.load_surfaces("ForestData", "branch-speak.til", 0, 17) # talking after getting hurt
        # self.foot1 = Resource.load_surfaces("ForestData", "foot1.cgf",0,0) # normal foot in trap
        # self.foot2 = Resource.load_surfaces("ForestData", "foot2.cgf",0,0) # red foot in trap
        # self.saks_die1 = Resource.load_surfaces("ForestData", "hgsaks.cgf",0,0) # foot in trap, eyes opened
        # self.saks_die2 = Resource.load_surfaces("ForestData", "hgsaks2.cgf", 0, 0 ) # foot in trap, eyes closed
        self.hugo_traptalk = Resource.load_surfaces("ForestData", "traptalk.til", 0, 15) # hurts talking
        self.hugo_traphurt = Resource.load_surfaces("ForestData", "trap-hurts.til", 0, 9) # hurts animation
        # self.trap_stars = Resource.load_surfaces("ForestData", "hurts.cgf", 0, 3) # stars
        self.score_numbers = Resource.load_surfaces("ForestData", "scores.cgf",0,0) # scores spritesheet
        self.hugo_lives = Resource.load_surfaces("ForestData", "hugostat.cgf",0,0) # life indicator
        self.sculla_hand1 = Resource.load_surfaces("ForestData", "heks1.cgf",0,0) # hand pressed
        self.sculla_hand2 = Resource.load_surfaces("ForestData", "heks2.cgf",0,0) # hand not pressed
        self.arrows = Resource.load_surfaces("ForestData", "arrows.cgf", 0, 4) # 2, 2 pressed, 8, 8 pressed

        self.sync_start = Resource.load_sync("ForestData", "005-01.oos")
        self.sync_rock = Resource.load_sync("ForestData", "005-02.oos")
        self.sync_dieonce = Resource.load_sync("ForestData", "005-03.oos")
        self.sync_trap = Resource.load_sync("ForestData", "005-04.oos")
        self.sync_lastlife = Resource.load_sync("ForestData", "005-05.oos")
        self.sync_catapult_talktop = Resource.load_sync("ForestData", "005-08.oos")
        self.sync_catapult_hang = Resource.load_sync("ForestData", "005-10.oos")
        self.sync_hitlog = Resource.load_sync("ForestData", "005-11.oos")
        self.sync_gameover = Resource.load_sync("ForestData", "005-12.oos")
        self.sync_levelcompleted = Resource.load_sync("ForestData", "005-13.oos")

        self.speak_start = Resource.load_speak("ForestData", "005-01.wav") # let's start
        self.speak_rock = Resource.load_speak("ForestData", "005-02.wav") # hit by rock
        self.speak_dieonce = Resource.load_speak("ForestData", "005-03.wav") # 2 more lives
        self.speak_trap = Resource.load_speak("ForestData", "005-04.wav") # hit by trap
        self.speak_lastlife = Resource.load_speak("ForestData", "005-05.wav") # last chance
        self.speak_catapult_up = Resource.load_speak("ForestData", "005-06.wav") # flying up scream
        self.speak_catapult_hit = Resource.load_speak("ForestData", "005-07.wav") # hit by catapult
        self.speak_catapult_talktop = Resource.load_speak("ForestData", "005-08.wav") # what are we doing here
        self.speak_catapult_down = Resource.load_speak("ForestData", "005-09.wav") # fall noise
        self.speak_catapult_hang = Resource.load_speak("ForestData", "005-10.wav") # time to jump
        self.speak_hitlog = Resource.load_speak("ForestData", "005-11.wav") # look at the birds
        self.speak_gameover = Resource.load_speak("ForestData", "005-12.wav") # this game is kaput
        self.speak_levelcompleted = Resource.load_speak("ForestData", "005-13.wav") # we did it quite well

        self.sfx_bg_atmosphere = Resource.load_sfx("ForestData", "atmos-lp.wav") # bg music
        # self.sfx_bg_atmosphere_evil = Resource.load_sfx("ForestData", "water-lp.wav")
        # self.sfx_trap_ring = Resource.load_sfx("ForestData", "riiing-lp.wav") # hitting trap
        self.sfx_lightning_warning = Resource.load_sfx("ForestData", "warning.wav") # hitting sylla button
        self.sfx_hugo_knock = Resource.load_sfx("ForestData", "knock.wav") # hitting screen
        self.sfx_hugo_hittrap = Resource.load_sfx("ForestData", "crunch.wav")  # hitting trap
        # self.sfx_lightning = Resource.load_sfx("ForestData", "lightning.wav")
        self.sfx_hugo_launch = Resource.load_sfx("ForestData", "skriid.wav")
        self.sfx_sack_normal = Resource.load_sfx("ForestData", "sack-norm.wav")
        self.sfx_sack_bonus = Resource.load_sfx("ForestData", "sack.wav")
        self.sfx_tree_swush = Resource.load_sfx("ForestData", "wush.wav")
        self.sfx_hugo_hitlog = Resource.load_sfx("ForestData", "bell.wav")
        self.sfx_catapult_eject = Resource.load_sfx("ForestData", "fjeder.wav")
        # self.sfx_birds = Resource.load_sfx("ForestData", "birds-lp.wav")
        # self.sfx_hugo_foothurt = Resource.load_sfx("ForestData", "timpani.wav")
        # self.sfx_hugo_hitscreen = Resource.load_sfx("ForestData", "hit_screen.wav")
        # self.sfx_hugo_screenklir = Resource.load_sfx("ForestData", "klirr.wav")
        self.sfx_hugo_crash = Resource.load_sfx("ForestData", "kineser.wav")
        self.sfx_hugo_hangstart = Resource.load_sfx("ForestData", "knage-start.wav")
        self.sfx_hugo_hang = Resource.load_sfx("ForestData", "knage-lp.wav")
        self.sfx_hugo_walk0 = Resource.load_sfx("ForestData", "fumle0.wav")
        self.sfx_hugo_walk1 = Resource.load_sfx("ForestData", "fumle1.wav")
        self.sfx_hugo_walk2 = Resource.load_sfx("ForestData", "fumle2.wav")
        self.sfx_hugo_walk3 = Resource.load_sfx("ForestData", "fumle3.wav")
        self.sfx_hugo_walk4 = Resource.load_sfx("ForestData", "fumle4.wav")
        # self.sfx_rockroll = Resource.load_sfx("ForestData", "rumle-lp.wav")

    def play(self, sound, sound_name, delay: float = 0 ):
        if time.time() - self.state_start > delay and sound_name not in self.played:
            pygame.mixer.Sound.play(sound)
            self.played[sound_name] = True

    def play_now(self, sound):
        pygame.mixer.Sound.play(sound)

    def render(self, screen):
        state_time = time.time() - self.state_start
        frame_index = int(state_time * 10)

        self.render_background(screen)
        self.render_bottom(screen)

        if self.state == ForestState.WAIT_INTRO:
            self.play(self.speak_start, "speak_start")

            if frame_index >= len(self.sync_start):
                self.switch_to(ForestState.PLAYING)
            else:
                actual_index = self.sync_start[frame_index]
                screen.blit(self.hugo_telllives[actual_index], (0,0))

        elif self.state == ForestState.PLAYING:
            self.played = {}

            fract, integer = math.modf(self.parallax_pos)
            integer = math.floor(integer) + 1
            if integer >= self.end_max_time:
                integer = self.end_max_time - 1

            for index in range(len(self.obstacles)):
                obstacle_pos = (index - self.parallax_pos) * self.ground_speed

                if self.obstacles[index] == 1: # Catapult
                    idx = frame_index % len(self.catapult)
                    dy = [45,43,39,34,29,22,14,1]
                    screen.blit(self.catapult[idx], (obstacle_pos - 8, 112 + dy[idx]))
                if self.obstacles[index] == 2: # Trap
                    idx = frame_index % len(self.trap)
                    dy = [176, 173, 169, 165, 176, 176]
                    screen.blit(self.trap[idx], (obstacle_pos - 8, dy[idx] - 24))
                if self.obstacles[index] == 3: # Rock
                    idx = frame_index % len(self.rock)
                    screen.blit(self.rock[idx], (obstacle_pos - math.sin(fract*(2*math.pi)) * 15, 120))
                if self.obstacles[index] == 4: # Tree
                    idx = frame_index % len(self.tree)
                    screen.blit(self.lone_tree[0], (obstacle_pos-52, -40))
                    screen.blit(self.tree[idx], (obstacle_pos, 52+10))

            for index in range(len(self.sacks)):
                sack_pos = (index - self.parallax_pos) * self.ground_speed

                if self.sacks[index] != 0:
                    screen.blit(self.sack[0], (sack_pos, 32))

            for index in range(len(self.leaves)):
                leave_pos = (index - self.parallax_pos) * self.ground_speed
                if self.leaves[index] == 1:
                    screen.blit(self.leaves2[0], (leave_pos, -10))
                elif self.leaves[index] == 2:
                    screen.blit(self.leaves1[0], (leave_pos, -10))

            if self.arrow_up_focus:
                screen.blit(self.arrows[1], (256 ,17))
            else:
                screen.blit(self.arrows[0], (256+2, 16+3))
            if self.arrow_down_focus:
                screen.blit(self.arrows[3], (256 ,54))
            else:
                screen.blit(self.arrows[2], (256+2, 54+2))

            if self.old_second == None:
                self.old_second = math.floor(self.parallax_pos)

            if self.old_second != math.floor(self.parallax_pos):
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

                if self.obstacles[integer] != 0:
                    if self.obstacles[integer] == 1 and not self.hugo_jumping:  # Catapult
                        self.play_now(self.speak_catapult_hit)
                        self.play_now(self.sfx_hugo_launch)
                        self.play_now(self.sfx_catapult_eject)
                        self.obstacles[integer] = 0
                        self.switch_to(ForestState.HURT_FLYING_START)
                    elif self.obstacles[integer] == 2 and not self.hugo_jumping:  # Trap
                        self.play_now(self.sfx_hugo_hittrap)
                        self.obstacles[integer] = 0
                        self.switch_to(ForestState.HURT_TRAP_ANIMATION)
                    elif self.obstacles[integer] == 3 and not self.hugo_jumping:  # Rock
                        self.play_now(self.sfx_hugo_hitlog)
                        self.obstacles[integer] = 0
                        self.switch_to(ForestState.HURT_ROCK_ANIMATION)
                    elif self.obstacles[integer] == 4:  # Tree
                        if self.hugo_crawling:
                            self.play_now(self.sfx_tree_swush)
                        else:
                            self.play_now(self.sfx_hugo_hitlog)
                            self.obstacles[integer] = 0
                            self.switch_to(ForestState.HURT_BRANCH_ANIMATION)

            if self.hugo_jumping:
                dy = -250*fract**2 + 250*fract - 22.5
                screen.blit(self.hugo_jump[frame_index % len(self.hugo_jump)], (0, 40 - dy))

                if self.sacks[integer] != 0:
                    if self.sacks[integer] == 1:
                        self.score += 100
                        self.play_now(self.sfx_sack_normal)
                        self.sacks[integer] = 0
                    elif self.sacks[integer] == 2:
                        self.score += 250
                        self.play_now(self.sfx_sack_bonus)
                        self.sacks[integer] = 0

            elif self.hugo_crawling:
                screen.blit(self.hugo_crawl[frame_index % len(self.hugo_crawl)], (0, 105))
            else:
                screen.blit(self.hugo_side[frame_index % len(self.hugo_side)], (0, 90))

            if frame_index % 8 == 0 and not self.hugo_jumping:
                walk_sfx = [self.sfx_hugo_walk0, self.sfx_hugo_walk1, self.sfx_hugo_walk2, self.sfx_hugo_walk3,
                            self.sfx_hugo_walk4]
                self.play_now(walk_sfx[random.randint(0, 4)])

            self.old_second = math.floor(self.parallax_pos)

        elif self.state == ForestState.SCYLLA_BUTTON:
            self.play(self.sfx_lightning_warning, "sfx_lightning_warning", 0.5)
            screen.blit(frame_index % 2 == 0 and self.sculla_hand1[0] or self.sculla_hand2[0])
            self.scylla_affected = True
            if state_time > 2:
                self.switch_to(ForestState.PLAYING)

        elif self.state == ForestState.HURT_ROCK_ANIMATION:
            if frame_index >= len(self.hugo_lookrock):
                self.switch_to(ForestState.HURT_ROCK_HIT_ANIMATION)
            else:
                screen.blit(self.hugo_lookrock[frame_index], (0,0))
                self.render_bottom( screen)

        elif self.state == ForestState.HURT_ROCK_HIT_ANIMATION:
            if frame_index >= len(self.hit_rock):
                self.switch_to(ForestState.HURT_ROCK_TALKING)
            else:
                screen.blit(self.hit_rock[frame_index], (0,0))
                self.render_bottom( screen)

        elif self.state == ForestState.HURT_ROCK_TALKING:
            self.play(self.speak_rock, "speak_rock")
            if frame_index >= len(self.sync_rock):
                self.reduce_lifes()
            else:
                actual_index = self.sync_rock[frame_index]
                screen.blit(self.hit_rock_sync[actual_index], (0,0))
                self.render_bottom(screen)

        elif self.state == ForestState.HURT_BRANCH_ANIMATION:
            if frame_index >= len(self.hugohitlog):
                self.switch_to(ForestState.HURT_BRANCH_TALKING)
            else:
                screen.blit(self.hugohitlog[frame_index], (0,0))
                self.render_bottom(screen)

        elif self.state == ForestState.HURT_BRANCH_TALKING:
            self.play(self.speak_hitlog, "speak_hitlog")
            if frame_index >= len(self.sync_hitlog):
                self.reduce_lifes()
            else:
                actual_index = self.sync_hitlog[frame_index]
                screen.blit(self.hugohitlog_talk[actual_index], (0,0))
                self.render_bottom(screen)

        elif self.state == ForestState.HURT_FLYING_START:
            self.play(self.speak_catapult_up, "speak_catapult_up")
            if frame_index >= len(self.catapult_fly):
                self.switch_to(ForestState.HURT_FLYING_TALKING)
            else:
                screen.blit(self.catapult_fly[frame_index], (0,0))

        elif self.state == ForestState.HURT_FLYING_TALKING:
            self.play(self.speak_catapult_talktop, "speak_catapult_talktop")
            if frame_index >= len(self.sync_catapult_talktop):
                self.switch_to(ForestState.HURT_FLYING_FALLING)
            else:
                actual_index = self.sync_catapult_talktop[frame_index]
                screen.blit(self.catapult_airtalk[actual_index], (0,0))

        elif self.state == ForestState.HURT_FLYING_FALLING:
            self.play(self.speak_catapult_down, "speak_catapult_down")
            self.play(self.sfx_hugo_crash, "sfx_hugo_crash")
            if frame_index >= len(self.catapult_fall):
                self.switch_to(ForestState.HURT_FLYING_FALLING_HANG_ANIMATION)
            else:
                screen.blit(self.catapult_fall[frame_index], (0,0))

        elif self.state == ForestState.HURT_FLYING_FALLING_HANG_ANIMATION:
            self.play(self.sfx_hugo_hangstart, "sfx_hugo_hangstart")
            if frame_index >= len(self.catapult_hang):
                self.switch_to(ForestState.HURT_FLYING_FALLING_HANG_TALKING)
            else:
                screen.blit(self.catapult_hang[frame_index], (0,0))

        elif self.state == ForestState.HURT_FLYING_FALLING_HANG_TALKING:
            self.play(self.speak_catapult_hang, "speak_catapult_hang")
            self.play(self.sfx_hugo_hang, "sfx_hugo_hang")
            if frame_index >= len(self.sync_catapult_hang):
                self.reduce_lifes()
            else:
                actual_index = self.sync_catapult_hang[frame_index]
                screen.blit(self.catapult_hang[12], (0,0))
                screen.blit(self.catapult_hangspeak[actual_index-1], (115, 117))

        elif self.state == ForestState.HURT_TRAP_ANIMATION:
            if frame_index >= len(self.hugo_traphurt):
                self.switch_to(ForestState.HURT_TRAP_TALKING)
            else:
                screen.blit(self.hugo_traphurt[frame_index], (0,0))

        elif self.state == ForestState.HURT_TRAP_TALKING:
            self.play(self.speak_trap, "speak_trap")
            if frame_index >= len(self.sync_trap):
                self.reduce_lifes()
            else:
                actual_index = self.sync_trap[frame_index]
                screen.blit(self.hugo_traptalk[actual_index], (0,0))

        elif self.state == ForestState.TALKING_AFTER_HURT:
            self.play(self.sfx_hugo_knock, "sfx_hugo_knock")
            self.play(self.sfx_hugo_knock, "sfx_hugo_knock_1", 0.25)
            self.play(self.sfx_hugo_knock, "sfx_hugo_knock_2", 0.5)

            if self.lifes == 1:
                self.play(self.speak_lastlife, "speak_lastlife")
                if frame_index >= len(self.sync_lastlife):
                    self.switch_to(ForestState.PLAYING)
                else:
                    actual_index = self.sync_lastlife[frame_index]
                    screen.blit(self.hugo_telllives[actual_index], (128, -16))
            else:
                self.play(self.speak_dieonce, "speak_dieonce")
                if frame_index >= len(self.sync_dieonce):
                    self.switch_to(ForestState.PLAYING)
                else:
                    actual_index = self.sync_dieonce[frame_index]
                    screen.blit(self.hugo_telllives[actual_index], (128, -16))

            if frame_index < 8:
                if frame_index % 2 == 0:
                    screen.blit(self.hugo_hand1[0], (96, 83))
                else:
                    screen.blit(self.hugo_hand2[0], (96, 78))

        elif self.state == ForestState.TALKING_GAME_OVER:
            self.play(self.speak_gameover, "speak_gameover")
            if frame_index >= len(self.sync_gameover):
                self.end()
            else:
                actual_index = self.sync_gameover[frame_index]
                screen.blit(self.hugo_telllives[actual_index], (128,-16))

        elif self.state == ForestState.WIN_DIALOGUE:
            self.play(self.speak_levelcompleted, "speak_levelcompleted")
            if frame_index >= len(self.sync_levelcompleted):
                self.end()
            else:
                actual_index = self.sync_levelcompleted[frame_index]
                screen.blit(self.hugo_telllives[actual_index], (128,-16))

    def reduce_lifes(self):
        self.lifes -= 1
        if self.lifes == 0:
            self.switch_to(ForestState.TALKING_GAME_OVER)
        else:
            self.switch_to(ForestState.TALKING_AFTER_HURT)

    def render_background(self, screen):
        hills_speed = 6
        trees_speed = 12
        grass_speed = 30
        hills_width = self.bg_hillsday[0].get_width()
        trees_width = self.bg_trees[0].get_width()
        grass_width = self.grass[0].get_width()
        ground_width = self.bg_ground[0].get_width()

        hills_pos = (self.new_mod(- self.parallax_pos * hills_speed, hills_width), 0)
        trees_pos = (self.new_mod(- self.parallax_pos * trees_speed, trees_width), -24)
        grass_pos = (self.new_mod(- self.parallax_pos * grass_speed, grass_width), 172)
        ground_pos = (self.new_mod(- self.parallax_pos * self.ground_speed, ground_width), 158)
        hills_pos_next = (hills_pos[0] + hills_width, hills_pos[1])
        tress_pos_next = (trees_pos[0] + trees_width, trees_pos[1])

        mountain_pos = 320 - 96 - (self.parallax_pos - self.end_max_time) * self.ground_speed

        screen.blit(self.bg_gradient, (0,0))
        screen.blit(self.bg_hillsday[0], hills_pos)
        screen.blit(self.bg_hillsday[0], hills_pos_next)

        if self.scylla_affected:
            screen.blit(self.bg_spooky_trees[0], trees_pos)
            screen.blit(self.bg_spooky_trees[0], tress_pos_next)
        else:
            screen.blit(self.bg_trees[0], trees_pos)
            screen.blit(self.bg_trees[0], tress_pos_next)

        for i in range(5):
            ground_x = ground_pos[0] + i * ground_width
            screen.blit(self.bg_ground[0], (ground_x, ground_pos[1]))

        for i in range(12):
            grass_x = grass_pos[0] + i * grass_width
            screen.blit(self.grass[0], (grass_x, grass_pos[1]))

        screen.blit(self.end_mountain[0], (mountain_pos, -16))

    def render_bottom(self, screen):
        screen.blit(self.scoreboard, (0, 184))
        for i in range(self.lifes):
            screen.blit(self.hugo_lives[0], (i * 40 + 32, 188))

        x_score = 200
        y_score = 194
        x_space = 24
        thousands = self.score // 1000
        hundreds = (self.score - thousands * 1000) // 100
        tens = (self.score - thousands * 1000 - hundreds * 100) // 10
        ones = self.score - thousands * 1000 - hundreds * 100 - tens * 10

        screen.blit(self.score_numbers[0], (x_score + x_space * 0, y_score), self.get_spritesheet_area(thousands))
        screen.blit(self.score_numbers[0], (x_score + x_space * 1, y_score), self.get_spritesheet_area(hundreds))
        screen.blit(self.score_numbers[0], (x_score + x_space * 2, y_score), self.get_spritesheet_area(tens))
        screen.blit(self.score_numbers[0], (x_score + x_space * 3, y_score), self.get_spritesheet_area(ones))

    def switch_to(self, new_state: ForestState):
        self.state = new_state
        self.state_start = time.time()

    @staticmethod
    def new_mod(a, b):
        res = a % b
        return res if not res else res - b if a < 0 else res

    def process_events(self, phone_events):
        if not self.background_music_playing:
            pygame.mixer.Sound.play(self.sfx_bg_atmosphere, loops=-1)
            self.background_music_playing = True

        if phone_events.press_2 and not self.arrow_down_focus:
            self.arrow_up_focus = True

        if phone_events.press_8 and not self.arrow_up_focus:
            self.arrow_down_focus = True

        if self.state is None:
            self.switch_to(ForestState.PLAYING)

        if self.last_time == None:
            self.last_time = time.time()

        if self.state == ForestState.PLAYING:
            self.parallax_pos += time.time() - self.last_time
            self.last_time = time.time()
        else:
            self.last_time = None

        if self.parallax_pos > self.end_max_time:
            self.parallax_pos = self.end_max_time
            self.switch_to(ForestState.WIN_DIALOGUE)


    def end(self):
        pygame.mixer.Sound.stop(self.sfx_bg_atmosphere)
        self.ended = True

    @staticmethod
    def get_spritesheet_area(value):
        width = 32
        height = 33

        xpos = value%5
        ypos = value//5

        return 1+xpos*(width+1), 1+ypos*(height+1), width, height

    def generate_obstacles(self):
        empty_prob = 0.65
        other_prob = (1 - empty_prob)/4
        out = random.choices([0, 1, 2, 3, 4], weights=[empty_prob, other_prob, other_prob, other_prob, other_prob], k=self.end_max_time)
        out[0] = 0
        out[1] = 0
        for idx in range(len(out)-1):
            if out[idx] != 0:
                out[idx+1] = 0

        return out

    def generate_sacks(self):
        empty_prob = 0.7
        other_prob = (1-empty_prob)*0.7
        other_prob_high = (1-empty_prob)*0.3
        return random.choices([0, 1, 2], weights=[empty_prob, other_prob, other_prob_high], k=self.end_max_time)

    def generate_leaves(self):
        arr = random.choices([1, 2], weights=[0.5, 0.5], k=self.end_max_time)
        for idx in range(len(arr)-1):
            if arr[idx] == 2:
                arr[idx+1] = 0
        return arr


