import pygame

from resource import Resource


class ForestResources:
    bg_hillsday = None
    # bg_hillsnight = None
    bg_trees = None
    bg_spooky_trees = None
    bg_ground = None
    grass = None
    leaves1 = None
    leaves2 = None
    scoreboard = None
    bg_gradient = None
    end_mountain = None
    # hugo_flyv = None
    hugo_side = None
    hugo_crawl = None
    hugo_jump = None
    # hugo_scared = None
    # hugo_end = None
    # hugo_intrap = None
    hugo_telllives = None
    hugo_hand1 = None
    hugo_hand2 = None
    tree = None
    lone_tree = None
    rock = None
    sack = None
    trap = None
    catapult = None
    hit_rock = None
    hugo_lookrock = None
    hit_rock_sync = None
    catapult_fly = None
    catapult_fall = None
    catapult_airtalk = None
    catapult_hang = None
    catapult_hangspeak = None
    hugohitlog = None
    hugohitlog_talk = None
    # foot1 = None
    # foot2 = None
    # saks_die1 = None
    # saks_die2 = None
    hugo_traptalk = None
    hugo_traphurt = None
    # trap_stars = None
    score_numbers = None
    hugo_lives = None
    sculla_hand1 = None
    sculla_hand2 = None
    arrows = None

    sync_start = None
    sync_rock = None
    sync_dieonce = None
    sync_trap = None
    sync_lastlife = None
    sync_catapult_talktop = None
    sync_catapult_hang = None
    sync_hitlog = None
    sync_gameover = None
    sync_levelcompleted = None

    speak_start = None
    speak_rock = None
    speak_dieonce = None
    speak_trap = None
    speak_lastlife = None
    speak_catapult_up = None
    speak_catapult_hit = None
    speak_catapult_talktop = None
    speak_catapult_down = None
    speak_catapult_hang = None
    speak_hitlog = None
    speak_gameover = None
    speak_levelcompleted = None

    sfx_bg_atmosphere = None
    # sfx_bg_atmosphere_evil = None
    # sfx_trap_ring = None
    sfx_lightning_warning = None
    sfx_hugo_knock = None
    sfx_hugo_hittrap = None
    # sfx_lightning = None
    sfx_hugo_launch = None
    sfx_sack_normal = None
    sfx_sack_bonus = None
    sfx_tree_swush = None
    sfx_hugo_hitlog = None
    sfx_catapult_eject = None
    sfx_birds = None
    # sfx_hugo_foothurt = None
    sfx_hugo_hitscreen = None
    sfx_hugo_screenklir = None
    sfx_hugo_crash = None
    sfx_hugo_hangstart = None
    sfx_hugo_hang = None
    sfx_hugo_walk0 = None
    sfx_hugo_walk1 = None
    sfx_hugo_walk2 = None
    sfx_hugo_walk3 = None
    sfx_hugo_walk4 = None

    # sfx_rockroll = None

    inverted_arrows = None

    @staticmethod
    def init():
        ForestResources.bg_hillsday = Resource.load_surfaces("ForestData", "hillsday.cgf", 0, 0)  # hills night
        # ForestResources.bg_hillsnight = Resource.load_surfaces("ForestData", "hillsngt.cgf" ,0,0) # hills day
        ForestResources.bg_trees = Resource.load_surfaces("ForestData", "paratrees.cgf", 0, 0)  # more background
        ForestResources.bg_spooky_trees = Resource.load_surfaces("ForestData", "spooky.cgf", 0, 0)  # far background
        ForestResources.bg_ground = Resource.load_surfaces("ForestData", "paraground.cgf", 0, 0)  # ground
        ForestResources.grass = Resource.load_surfaces("ForestData", "grass.cgf", 0, 0)  # near glass
        ForestResources.leaves1 = Resource.load_surfaces("ForestData", "leaves1.cgf", 0, 0)  # bigger leaves
        ForestResources.leaves2 = Resource.load_surfaces("ForestData", "leaves2.cgf", 0, 0)  # upper leaves
        ForestResources.scoreboard = Resource.load_surface_raw("ForestData", "scorebrd.bmp")  # lower portion
        ForestResources.bg_gradient = pygame.image.load("resources/fixed_assets/gradient.bmp")  # background gradient
        ForestResources.end_mountain = Resource.load_surfaces("ForestData", "wall.cgf", 0, 0)  # ending wall
        # ForestResources.hugo_flyv = Resource.load_surfaces("ForestData", "hugoflyv.cgf", 0,0) # looking up preocupated, hit by branch
        ForestResources.hugo_side = Resource.load_surfaces("ForestData", "hugoside.cgf", 0, 7)  # hugo walking
        ForestResources.hugo_crawl = Resource.load_surfaces("ForestData", "kravle.cgf", 0, 7)  # crawls
        ForestResources.hugo_jump = Resource.load_surfaces("ForestData", "hugohop.cgf", 0, 2)  # hugo jumps
        # ForestResources.hugo_scared = Resource.load_surfaces("ForestData", "scared.cgf", 0,7) # scared, when?
        # ForestResources.hugo_end = Resource.load_surfaces("ForestData", "hugoend.cgf", 0,0) # looks at camera after ending
        # ForestResources.hugo_intrap = Resource.load_surfaces("ForestData", "caught.cgf",0,0) # in trap
        ForestResources.hugo_telllives = Resource.load_surfaces("ForestData", "hugo_hello.cgf", 0,
                                                                15)  # hugo tells remaining lives
        ForestResources.hugo_hand1 = Resource.load_surfaces("ForestData", "hand1.cgf", 0, 0)  # hand far
        ForestResources.hugo_hand2 = Resource.load_surfaces("ForestData", "hand2.cgf", 0, 0)  # hand closer
        ForestResources.tree = Resource.load_surfaces("ForestData", "branch-swing.cgf", 0, 6)  # branch swinging
        ForestResources.lone_tree = Resource.load_surfaces("ForestData", "lonetree.cgf", 0, 0)  # single tree
        ForestResources.rock = Resource.load_surfaces("ForestData", "stone.cgf", 0, 7)  # rotating boulder
        ForestResources.sack = Resource.load_surfaces("ForestData", "saek.cgf", 0, 3)  # bags
        ForestResources.trap = Resource.load_surfaces("ForestData", "faelde.cgf", 0, 5)  # trap in floor
        ForestResources.catapult = Resource.load_surfaces("ForestData", "catapult.cgf", 0, 7)  # catapult in floor
        ForestResources.hit_rock = Resource.load_surfaces("ForestData", "hgrock.til", 0, 60)  # hit by ball
        ForestResources.hugo_lookrock = Resource.load_surfaces("ForestData", "hugo-rock.til", 0, 14)  # almost crushed
        ForestResources.hit_rock_sync = Resource.load_surfaces("ForestData", "msyncrck.til", 0,
                                                               17)  # talking after hit by ball
        ForestResources.catapult_fly = Resource.load_surfaces("ForestData", "hgkatfly.til", 0, 113)  # flying in trap
        ForestResources.catapult_fall = Resource.load_surfaces("ForestData", "hgkatfly.til", 115,
                                                               189)  # falling in trap
        ForestResources.catapult_airtalk = Resource.load_surfaces("ForestData", "catapult-speak.til", 0,
                                                                  15)  # talking in air
        ForestResources.catapult_hang = Resource.load_surfaces("ForestData", "hgkathng.til", 0,
                                                               12)  # hanging from branch
        ForestResources.catapult_hangspeak = Resource.load_surfaces("ForestData", "hanging_mouth.cgf", 0,
                                                                    11)  # mouth parts when hanging
        ForestResources.hugohitlog = Resource.load_surfaces("ForestData", "branch-groggy.til", 0,
                                                            42)  # hurt with branch
        ForestResources.hugohitlog_talk = Resource.load_surfaces("ForestData", "branch-speak.til", 0,
                                                                 17)  # talking after getting hurt
        # ForestResources.foot1 = Resource.load_surfaces("ForestData", "foot1.cgf",0,0) # normal foot in trap
        # ForestResources.foot2 = Resource.load_surfaces("ForestData", "foot2.cgf",0,0) # red foot in trap
        # ForestResources.saks_die1 = Resource.load_surfaces("ForestData", "hgsaks.cgf",0,0) # foot in trap, eyes opened
        # ForestResources.saks_die2 = Resource.load_surfaces("ForestData", "hgsaks2.cgf", 0, 0 ) # foot in trap, eyes closed
        ForestResources.hugo_traptalk = Resource.load_surfaces("ForestData", "traptalk.til", 0, 15)  # hurts talking
        ForestResources.hugo_traphurt = Resource.load_surfaces("ForestData", "trap-hurts.til", 0, 9)  # hurts animation
        # ForestResources.trap_stars = Resource.load_surfaces("ForestData", "hurts.cgf", 0, 3) # stars
        ForestResources.score_numbers = Resource.load_surfaces("ForestData", "scores.cgf", 0, 0)  # scores spritesheet
        ForestResources.hugo_lives = Resource.load_surfaces("ForestData", "hugostat.cgf", 0, 0)  # life indicator
        ForestResources.sculla_hand1 = Resource.load_surfaces("ForestData", "heks1.cgf", 0, 0)  # hand pressed
        ForestResources.sculla_hand2 = Resource.load_surfaces("ForestData", "heks2.cgf", 0, 0)  # hand not pressed
        ForestResources.arrows = [pygame.image.load("resources/fixed_assets/arrows.cgf_" + str(i) + ".png") for i in range(4)] # 2, 2 pressed, 8, 8 pressed

        ForestResources.sync_start = Resource.load_sync("ForestData", "005-01.oos")
        ForestResources.sync_rock = Resource.load_sync("ForestData", "005-02.oos")
        ForestResources.sync_dieonce = Resource.load_sync("ForestData", "005-03.oos")
        ForestResources.sync_trap = Resource.load_sync("ForestData", "005-04.oos")
        ForestResources.sync_lastlife = Resource.load_sync("ForestData", "005-05.oos")
        ForestResources.sync_catapult_talktop = Resource.load_sync("ForestData", "005-08.oos")
        ForestResources.sync_catapult_hang = Resource.load_sync("ForestData", "005-10.oos")
        ForestResources.sync_hitlog = Resource.load_sync("ForestData", "005-11.oos")
        ForestResources.sync_gameover = Resource.load_sync("ForestData", "005-12.oos")
        ForestResources.sync_levelcompleted = Resource.load_sync("ForestData", "005-13.oos")

        ForestResources.speak_start = Resource.load_speak("ForestData", "005-01.wav")  # let's start
        ForestResources.speak_rock = Resource.load_speak("ForestData", "005-02.wav")  # hit by rock
        ForestResources.speak_dieonce = Resource.load_speak("ForestData", "005-03.wav")  # 2 more lives
        ForestResources.speak_trap = Resource.load_speak("ForestData", "005-04.wav")  # hit by trap
        ForestResources.speak_lastlife = Resource.load_speak("ForestData", "005-05.wav")  # last chance
        ForestResources.speak_catapult_up = Resource.load_speak("ForestData", "005-06.wav")  # flying up scream
        ForestResources.speak_catapult_hit = Resource.load_speak("ForestData", "005-07.wav")  # hit by catapult
        ForestResources.speak_catapult_talktop = Resource.load_speak("ForestData",
                                                                     "005-08.wav")  # what are we doing here
        ForestResources.speak_catapult_down = Resource.load_speak("ForestData", "005-09.wav")  # fall noise
        ForestResources.speak_catapult_hang = Resource.load_speak("ForestData", "005-10.wav")  # time to jump
        ForestResources.speak_hitlog = Resource.load_speak("ForestData", "005-11.wav")  # look at the birds
        ForestResources.speak_gameover = Resource.load_speak("ForestData", "005-12.wav")  # this game is kaput
        ForestResources.speak_levelcompleted = Resource.load_speak("ForestData", "005-13.wav")  # we did it quite well

        ForestResources.sfx_bg_atmosphere = Resource.load_sfx("ForestData", "atmos-lp.wav")  # bg music
        # ForestResources.sfx_bg_atmosphere_evil = Resource.load_sfx("ForestData", "water-lp.wav")
        # ForestResources.sfx_trap_ring = Resource.load_sfx("ForestData", "riiing-lp.wav") # hitting trap
        ForestResources.sfx_lightning_warning = Resource.load_sfx("ForestData", "warning.wav")  # hitting sylla button
        ForestResources.sfx_hugo_knock = Resource.load_sfx("ForestData", "knock.wav")  # hitting screen
        ForestResources.sfx_hugo_hittrap = Resource.load_sfx("ForestData", "crunch.wav")  # hitting trap
        # ForestResources.sfx_lightning = Resource.load_sfx("ForestData", "lightning.wav")
        ForestResources.sfx_hugo_launch = Resource.load_sfx("ForestData", "skriid.wav")
        ForestResources.sfx_sack_normal = Resource.load_sfx("ForestData", "sack-norm.wav")
        ForestResources.sfx_sack_bonus = Resource.load_sfx("ForestData", "sack.wav")
        ForestResources.sfx_tree_swush = Resource.load_sfx("ForestData", "wush.wav")
        ForestResources.sfx_hugo_hitlog = Resource.load_sfx("ForestData", "bell.wav")
        ForestResources.sfx_catapult_eject = Resource.load_sfx("ForestData", "fjeder.wav")
        ForestResources.sfx_birds = Resource.load_sfx("ForestData", "birds-lp.wav")
        # ForestResources.sfx_hugo_foothurt = Resource.load_sfx("ForestData", "timpani.wav")
        ForestResources.sfx_hugo_hitscreen = Resource.load_sfx("ForestData", "hit_screen.wav")
        ForestResources.sfx_hugo_screenklir = Resource.load_sfx("ForestData", "klirr.wav")
        ForestResources.sfx_hugo_crash = Resource.load_sfx("ForestData", "kineser.wav")
        ForestResources.sfx_hugo_hangstart = Resource.load_sfx("ForestData", "knage-start.wav")
        ForestResources.sfx_hugo_hang = Resource.load_sfx("ForestData", "knage-lp.wav")
        ForestResources.sfx_hugo_walk0 = Resource.load_sfx("ForestData", "fumle0.wav")
        ForestResources.sfx_hugo_walk1 = Resource.load_sfx("ForestData", "fumle1.wav")
        ForestResources.sfx_hugo_walk2 = Resource.load_sfx("ForestData", "fumle2.wav")
        ForestResources.sfx_hugo_walk3 = Resource.load_sfx("ForestData", "fumle3.wav")
        ForestResources.sfx_hugo_walk4 = Resource.load_sfx("ForestData", "fumle4.wav")
        # ForestResources.sfx_rockroll = Resource.load_sfx("ForestData", "rumle-lp.wav")

        ForestResources.inverted_arrows = pygame.image.load("resources/fixed_assets/inverted_arrows.png").convert_alpha()