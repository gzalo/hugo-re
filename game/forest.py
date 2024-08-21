from enum import Enum
from resource import Resource

class ForestEnum(Enum):
    WAIT_INTRO = 0
    PLAYING = 1
    SCYLLA_BUTTON = 2
    HURT_TRAP_ANIMATION = 3
    HURT_TRAP_TALKING = 4
    HURT_FLYING_START = 5
    HURT_FLYING_TALKING = 6
    HURT_FLYING_FALLING = 7
    HURT_ROCK_ANIMATION = 8
    HURT_ROCK_TALKING = 9
    HURT_BRANCH = 10
    TALKING_AFTER_HURT = 11
    TALKING_GAME_OVER = 12
    WIN_DIALOGUE = 13

class Forest:
    state = None
    state_start = None

    def __init__(self):
        self.score = 0
        self.ended = False
        self.bg_hillsday = Resource.load_surfaces("hillsday.cgf", 0,0) # hills night
        self.bg_hillsnight = Resource.load_surfaces("hillsngt.cgf" ,0,0) # hills day
        self.bg_trees = Resource.load_surfaces("paratrees.cgf" ,0,0) # more background
        self.bg_spooky_trees = Resource.load_surfaces("spooky.cgf", 0,0) # far background
        self.bg_ground = Resource.load_surfaces("paraground.cgf",0,0) # ground
        self.grass = Resource.load_surfaces("grass.cgf", 0,0) # near glass
        self.leaves1 = Resource.load_surfaces("leaves1.cgf",0,0)  # bigger leaves
        self.leaves2 = Resource.load_surfaces("leaves2.cgf",0,0) # upper leaves
        self.scoreboard = Resource.load_surfaces("scorebrd.bmp",0,0) # lower portion
        self.end_mountain = Resource.load_surfaces("wall.cgf",0,0) # ending wall
        self.hugo_flyv = Resource.load_surfaces("hugoflyv.cgf", 0,0) # looking up preocupated
        self.hugo_side = Resource.load_surfaces("hugoside.cgf", 0, 7) # hugo walking
        self.hugo_crawl = Resource.load_surfaces("kravle.cgf", 0, 7) # crawls
        self.hugo_jump = Resource.load_surfaces("hugohop.cgf", 0,2) # hugo jumps
        self.hugo_scared = Resource.load_surfaces("scared.cgf", 0,7) # scared, when?
        self.hugo_end = Resource.load_surfaces("hugoend.cgf", 0,0) # looks at camera after ending
        self.hugo_intrap = Resource.load_surfaces("caught.cgf",0,0) # in trap
        self.hugo_telllives = Resource.load_surfaces("hugo_hello.cgf",0,15) # hugo tells remaining lives
        self.hugo_hand1 = Resource.load_surfaces("hand1.cgf",0,0) # hand far
        self.hugo_hand2 = Resource.load_surfaces("hand2.cgf",0,0) # hand closer
        self.tree = Resource.load_surfaces("branch-swing.cgf",0,6) # branch swinging
        self.lone_tree = Resource.load_surfaces("lonetree.cgf", 0,0) # single tree
        self.rock = Resource.load_surfaces("stone.cgf",0,7) # rotating boulder
        self.sack = Resource.load_surfaces("saek.cgf", 0, 3) # bags
        self.trap = Resource.load_surfaces("faelde.cgf", 0, 5) # trap in floor
        self.catapult = Resource.load_surfaces("catapult.cgf", 0, 7) # catapult in floor
        self.hit_rock = Resource.load_surfaces("hgrock.til", 0, 60) # hit by ball
        self.hugo_lookrock = Resource.load_surfaces("hugo-rock.til", 0, 16) # almost crushed
        self.hit_rock_sync = Resource.load_surfaces("msyncrck.til", 0, 7) # talking after hit by ball

        self.catapult_fly = Resource.load_surfaces("hgkatfly.til", 0, 191) # flying in trap
        self.catapult_airtalk = Resource.load_surfaces("catapult-speak.til", 0, 15)  # talking in air
        self.catapult_hang = Resource.load_surfaces("hgkathng.til", 0, 12) # hanging from branch
        self.catapult_hangspeak = Resource.load_surfaces("hanging_mouth.cgf",0,11) # mouth parts when hanging
        self.hugohitlog = Resource.load_surfaces("branch-groggy.til",0,42) # hurt with branch
        self.hugohitlog_talk = Resource.load_surfaces("branch-speak.til", 0, 17) # talking after getting hurt
        self.foot1 = Resource.load_surfaces("foot1.cgf",0,0) # normal foot in trap
        self.foot2 = Resource.load_surfaces("foot2.cgf",0,0) # red foot in trap
        self.saks_die1 = Resource.load_surfaces("hgsaks.cgf",0,0) # foot in trap, eyes opened
        self.saks_die2 = Resource.load_surfaces("hgsaks2.cgf", 0, 0 ) # foot in trap, eyes closed
        self.hugo_traptalk = Resource.load_surfaces("traptalk.til", 0, 15) # hurts talking
        self.hugo_traphurt = Resource.load_surfaces("trap-hurts.til", 0, 9) # hurts animation
        self.trap_stars = Resource.load_surfaces("hurts.cgf", 0, 3) # stars
        self.score_numbers = Resource.load_surfaces("scores.cgf",0,0) # scores spritesheet
        self.hugo_lives = Resource.load_surfaces("hugostat.cgf",0,0) # life indicator
        self.sculla_hand1 = Resource.load_surfaces("heks1.cgf",0,0) # hand pressed
        self.sculla_hand2 = Resource.load_surfaces("heks2.cgf",0,0) # hand not pressed
        self.arrows = Resource.load_surfaces("arrows.cgf", 0, 4) # 2, 2 pressed, 8, 8 pressed

        self.sync_start = Resource.load_sync("005-01.oos")
        self.sync_rock = Resource.load_sync("005-02.oos")
        self.sync_dieonce = Resource.load_sync("005-03.oos")
        self.sync_trap = Resource.load_sync("005-04.oos")
        self.sync_lastlife = Resource.load_sync("005-05.oos")
        self.sync_catapult_talktop = Resource.load_sync("005-08.oos")
        self.sync_catapult_hang = Resource.load_sync("005-10.oos")
        self.sync_hitlog = Resource.load_sync("005-11.oos")
        self.sync_gameover = Resource.load_sync("005-12.oos")
        self.sync_levelcompleted = Resource.load_sync("005-13.oos")

        self.speak_start = Resource.load_speak("005-01.wav") # let's start
        self.speak_rock = Resource.load_speak("005-02.wav") # hit by rock
        self.speak_dieonce = Resource.load_speak("005-03.wav") # 2 more lives
        self.speak_trap = Resource.load_speak("005-04.wav") # hit by trap
        self.speak_lastlife = Resource.load_speak("005-05.wav") # last chance
        self.speak_catapult_up = Resource.load_speak("005-06.wav") # flying up scream
        self.speak_catapult_hit = Resource.load_speak("005-07.wav") # hit by catapult
        self.speak_catapult_talktop = Resource.load_speak("005-08.wav") # what are we doing here
        self.speak_catapult_down = Resource.load_speak("005-09.wav") # fall noise
        self.speak_catapult_hang = Resource.load_speak("005-10.wav") # time to jump
        self.speak_hitlog = Resource.load_speak("005-11.wav") # look at the birds
        self.speak_gameover = Resource.load_speak("005-12.wav") # this game is kaput
        self.speak_levelcompleted = Resource.load_speak("005-13.wav") # we did it quite well

        self.sfx_bg_atmosphere = Resource.load_sfx("atmos-lp.wav")
        self.sfx_bg_atmosphere_evil = Resource.load_sfx("water-lp.wav")
        self.sfx_trap_ring = Resource.load_sfx("riiing-lp.wav")
        self.sfx_lightning_warning = Resource.load_sfx("warning.wav")
        self.sfx_hugo_knock = Resource.load_sfx("knock.wav")
        self.sfx_hugo_hittrap = Resource.load_sfx("crunch.wav")
        self.sfx_lightning = Resource.load_sfx("lightning.wav")
        self.sfx_hugo_launch = Resource.load_sfx("skriid.wav")
        self.sfx_sack_normal = Resource.load_sfx("sack-norm.wav")
        self.sfx_sack_bonus = Resource.load_sfx("sack.wav")
        self.sfx_tree_swush = Resource.load_sfx("wush.wav")
        self.sfx_hugo_hitlog = Resource.load_sfx("bell.wav")
        self.sfx_catapult_eject = Resource.load_sfx("fjeder.wav")
        self.sfx_birds = Resource.load_sfx("birds-lp.wav")
        self.sfx_hugo_foothurt = Resource.load_sfx("timpani.wav")
        self.sfx_hugo_hitscreen = Resource.load_sfx("hit_screen.wav")
        self.sfx_hugo_screenklir = Resource.load_sfx("klirr.wav")
        self.sfx_hugo_crash = Resource.load_sfx("kineser.wav")
        self.sfx_hugo_hangstart = Resource.load_sfx("knage-start.wav")
        self.sfx_hugo_hang = Resource.load_sfx("knage-lp.wav")
        self.sfx_hugo_walk0 = Resource.load_sfx("fumle0.wav")
        self.sfx_hugo_walk1 = Resource.load_sfx("fumle1.wav")
        self.sfx_hugo_walk2 = Resource.load_sfx("fumle2.wav")
        self.sfx_hugo_walk3 = Resource.load_sfx("fumle3.wav")
        self.sfx_hugo_walk4 = Resource.load_sfx("fumle4.wav")
        self.sfx_rockroll = Resource.load_sfx("rumle-lp.wav")

    def render(self, screen):
        pass

    def process_events(self, phone_events):
        pass

    def end(self):
        pass