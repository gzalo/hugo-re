from enum import Enum

import random
import time

from phone_events import PhoneEvents
from resource import Resource
import pygame

class CaveState(Enum):
    WAITING_BEFORE_TALKING = 0
    TALKING_BEFORE_CLIMB = 1
    CLIMBING = 2
    WAITING_INPUT = 3
    GOING_FIRST = 4
    GOING_SECOND = 5
    GOING_THIRD = 6
    LOST_FIRST = 7
    LOST_SECOND = 8
    LOST_THIRD = 9
    LOST_SPRING = 10
    SCYLLA_LEAVES = 11
    SCYLLA_BIRD = 12
    SCYLLA_ROPES = 13
    SCYLLA_SPRING = 14
    FAMILY_CAGE_OPENS = 15
    FAMILY_HAPPY = 16

    def next(self):
        v = self.value + 1
        if v > len(CaveState) - 1:
            v = len(CaveState) - 1
        return CaveState(v)

    def previous(self):
        v = self.value - 1
        if v < 0:
            v = 0
        return CaveState(v)

class Cave:
    state: CaveState
    state_start = None
    hugo_sprite_pos = [
        (25, 105),
        (97, 100),
        (172, 102),
    ]
    argentine_version = True

    def __init__(self, score):
        caselive_til = "caselive.TIL"
        casedie_til = "casedie.TIL"
        self.talks = Resource.load_surfaces("RopeOutroData", "STAIRS.TIL", 0, 12)
        self.climbs = Resource.load_surfaces("RopeOutroData", "STAIRS.TIL", 11, 51)
        self.first_rope = Resource.load_surfaces("RopeOutroData", caselive_til, 0, 32)
        self.second_rope = Resource.load_surfaces("RopeOutroData", caselive_til, 33, 72)
        self.third_rope = Resource.load_surfaces("RopeOutroData", caselive_til, 73, 121)
        self.scylla_leaves = Resource.load_surfaces("RopeOutroData", caselive_til, 122, 177)
        self.scylla_bird = Resource.load_surfaces("RopeOutroData", caselive_til, 178, 240)
        self.scylla_ropes = Resource.load_surfaces("RopeOutroData", caselive_til, 241, 283)
        self.scylla_spring = Resource.load_surfaces("RopeOutroData", caselive_til, 284, 318)
        self.family_cage = Resource.load_surfaces("RopeOutroData", caselive_til, 319, 352)
        self.hugo_puff_first = Resource.load_surfaces("RopeOutroData", casedie_til, 122, 166)
        self.hugo_puff_second = Resource.load_surfaces("RopeOutroData", casedie_til, 167, 211)
        self.hugo_puff_third = Resource.load_surfaces("RopeOutroData", casedie_til, 212, 256)
        self.hugo_spring = Resource.load_surfaces("RopeOutroData", casedie_til, 257, 295)
        self.happy = Resource.load_surfaces("RopeOutroData", "happy.TIL", 0, 111)
        self.score_font = Resource.load_surfaces("RopeOutroData", "score.cgf", 0, 9)
        self.hugo = Resource.load_surfaces("RopeOutroData", "hugo.cgf", 0, 0)

        self.sync_hugo_start = Resource.load_sync("RopeOutroData", "002-06.oos")
        self.sync_hugo_die = Resource.load_sync("RopeOutroData", "002-09.oos")

        self.her_er_vi = Resource.load_speak("RopeOutroData", "002-05.wav") # Help us Hugo
        self.trappe_snak = Resource.load_speak("RopeOutroData", "002-06.wav")  # I feel like this is dangerous
        self.nu_kommer_jeg = Resource.load_speak("RopeOutroData", "002-07.wav") # Hello is there anyone here
        self.afskylia_snak = Resource.load_speak("RopeOutroData", "002-08.wav") # Go ahead and choose
        self.hugo_katapult = Resource.load_speak("RopeOutroData", "002-09.wav") # I'll be back
        self.hugo_skyd_ud = Resource.load_speak("RopeOutroData", "002-10.wav") # Hugo screams on lost
        self.afskylia_skyd_ud = Resource.load_speak("RopeOutroData", "002-11.wav") # Scylla screams on win
        self.hugoline_tak = Resource.load_speak("RopeOutroData", "002-12.wav") # We are free

        self.stemning = Resource.load_sfx("RopeOutroData", "ba-13.wav") # Background chimes
        self.fodtrin1 = Resource.load_sfx("RopeOutroData", "ba-15.wav") # Step 1
        self.fodtrin2 = Resource.load_sfx("RopeOutroData", "ba-16.wav") # Step 2
        self.hiv_i_reb = Resource.load_sfx("RopeOutroData", "ba-17.wav") # Rope used
        self.fjeder = Resource.load_sfx("RopeOutroData", "ba-18.wav") # Spring
        self.pre_puf = Resource.load_sfx("RopeOutroData", "ba-21.wav") # Anticipation
        self.puf = Resource.load_sfx("RopeOutroData", "ba-22.wav") # Smoke puff
        self.tast_trykket = Resource.load_sfx("RopeOutroData", "ba-24.wav") # Waiting for you ding
        self.pre_fanfare = Resource.load_sfx("RopeOutroData", "ba-101.wav") # Choose something background
        self.fanfare = Resource.load_sfx("RopeOutroData", "ba-102.wav") # Correct ending (This was starting to ...)
        self.fugle_skrig = Resource.load_sfx("RopeOutroData", "ba-104.wav") # Scylla turned into bird
        self.trappe_grin = Resource.load_sfx("RopeOutroData", "hexhaha.wav") # Odd laugh
        self.skrig = Resource.load_sfx("RopeOutroData", "skrig.wav") # Scilla screams
        self.score_counter = Resource.load_sfx("RopeOutroData", "counter.wav") # Score counter increase
        self.bg_music = Resource.load_sfx("RopeOutroData", "a_party_for_me.mp3") # BG music, downloaded from https://www.youtube.com/watch?v=maJVRkefBhw

        self.played = {}
        self.switch_to(CaveState.WAITING_BEFORE_TALKING)

        self.display_surface = None
        self.ended = False

        self.score = score
        self.rolling_score = self.score

        self.sounding_score = False
        self.playing_music = False
        self.selected_rope = None

    def process_events(self, phone_events: PhoneEvents):
        if self.state == CaveState.WAITING_INPUT:
            if phone_events.press_3:
                self.switch_to(CaveState.GOING_FIRST)
                pygame.mixer.Sound.play(self.tast_trykket)
                self.selected_rope = 0
            if phone_events.press_6:
                self.switch_to(CaveState.GOING_SECOND)
                pygame.mixer.Sound.play(self.tast_trykket)
                self.selected_rope = 1
            if phone_events.press_9:
                self.switch_to(CaveState.GOING_THIRD)
                pygame.mixer.Sound.play(self.tast_trykket)
                self.selected_rope = 2

    def switch_to(self, new_state: CaveState):
        self.state = new_state
        self.state_start = time.time()

    def play(self, sound, sound_name, delay: float = 0 ):
        if time.time() - self.state_start > delay and sound_name not in self.played:
            pygame.mixer.Sound.play(sound)
            self.played[sound_name] = True

    def render(self, screen):
        state_time = time.time() - self.state_start
        frame_index = int(state_time * 10)

        if self.state == CaveState.WAITING_BEFORE_TALKING:
            self.play(self.her_er_vi, "HerErVi")
            if self.argentine_version:
                self.play(self.bg_music, "BgMusic")
            else:
                self.play(self.stemning, "Stemning")
            self.playing_music = True

            if state_time > 2.5:
                self.switch_to(CaveState.TALKING_BEFORE_CLIMB)

            self.display_surface = self.talks[12]

        elif self.state == CaveState.TALKING_BEFORE_CLIMB:
            self.play(self.trappe_snak, "TrappeSnak")
            if not self.argentine_version:
                self.play(self.trappe_grin, "TrappeGrin", 4)

            if frame_index >= len(self.sync_hugo_start):
                self.switch_to(CaveState.CLIMBING)
            else:
                actual_index = self.sync_hugo_start[frame_index]
                self.display_surface = self.talks[actual_index]

        elif self.state == CaveState.CLIMBING:
            self.play(self.nu_kommer_jeg, "NuKommerJeg", 1)
            if not self.argentine_version:
                self.play(self.pre_fanfare, "PreFanfare", 2)

            if frame_index >= len(self.climbs):
                self.switch_to(CaveState.WAITING_INPUT)
            else:
                self.display_surface = self.climbs[frame_index]

        elif self.state == CaveState.WAITING_INPUT:
            if not self.argentine_version:
                self.play(self.stemning, "Stemning")
            self.play(self.afskylia_snak, "AfskyliaSnak", 0.5)
            self.display_surface = self.first_rope[0]

        elif self.state == CaveState.GOING_FIRST:
            if frame_index >= len(self.first_rope):
                self.calc_win_or_lose()
            else:
                self.display_surface = self.first_rope[frame_index]

            self.play_steps(frame_index)
            self.play(self.hiv_i_reb, "HivIReb", 2)

        elif self.state == CaveState.GOING_SECOND:
            if frame_index >= len(self.second_rope):
                self.calc_win_or_lose()
            else:
                self.display_surface = self.second_rope[frame_index]

            self.play_steps(frame_index)
            self.play(self.hiv_i_reb, "HivIReb", 3)

        elif self.state == CaveState.GOING_THIRD:
            if frame_index >= len(self.third_rope):
                self.calc_win_or_lose()
            else:
                self.display_surface = self.third_rope[frame_index]

            self.play_steps(frame_index)
            self.play(self.hiv_i_reb, "HivIReb", 4)

        elif self.state == CaveState.LOST_FIRST:
            if frame_index >= len(self.hugo_puff_first):
                self.switch_to(CaveState.LOST_SPRING)
            else:
                self.display_surface = self.hugo_puff_first[frame_index]

            self.play(self.pre_puf, "PrePuf", 1)
            self.play(self.puf, "Puf", 2)

        elif self.state == CaveState.LOST_SECOND:
            if frame_index >= len(self.hugo_puff_second):
                self.switch_to(CaveState.LOST_SPRING)
            else:
                self.display_surface = self.hugo_puff_second[frame_index]

            self.play(self.pre_puf, "PrePuf", 1)
            self.play(self.puf, "Puf", 2)

        elif self.state == CaveState.LOST_THIRD:
            if frame_index >= len(self.hugo_puff_third):
                self.switch_to(CaveState.LOST_SPRING)
            else:
                self.display_surface = self.hugo_puff_third[frame_index]

            self.play(self.pre_puf, "PrePuf", 1)
            self.play(self.puf, "Puf", 2)

        elif self.state == CaveState.LOST_SPRING:
            if frame_index >= len(self.hugo_spring):
                self.end()
            else:
                self.display_surface = self.hugo_spring[frame_index]

            self.play(self.hugo_katapult, "HugoKatapult")
            self.play(self.hugo_skyd_ud, "HugoSkydUd", 2.5)

        elif self.state == CaveState.SCYLLA_LEAVES:
            if frame_index >= len(self.scylla_leaves):
                self.switch_to(CaveState.FAMILY_CAGE_OPENS)
            else:
                self.display_surface = self.scylla_leaves[frame_index]

            self.play(self.skrig, "Skrig", 0.5)

        elif self.state == CaveState.SCYLLA_BIRD:
            if frame_index >= len(self.scylla_bird):
                self.switch_to(CaveState.FAMILY_CAGE_OPENS)
            else:
                self.display_surface = self.scylla_bird[frame_index]

            self.play(self.fugle_skrig, "FugleSkrig", 0.5)

        elif self.state == CaveState.SCYLLA_ROPES:
            if frame_index >= len(self.scylla_ropes):
                self.switch_to(CaveState.SCYLLA_SPRING)
            else:
                self.display_surface = self.scylla_ropes[frame_index]

            self.play(self.pre_puf, "PrePuf", 1)
            self.play(self.puf, "Puf", 2)

        elif self.state == CaveState.SCYLLA_SPRING:
            if frame_index >= len(self.scylla_spring):
                self.switch_to(CaveState.FAMILY_CAGE_OPENS)
            else:
                self.display_surface = self.scylla_spring[frame_index]

            self.play(self.fjeder, "Fjeder", 1.5)
            self.play(self.afskylia_skyd_ud, "AfskyliaSkydUd", 2)

        elif self.state == CaveState.FAMILY_CAGE_OPENS:
            if frame_index >= len(self.family_cage):
                self.switch_to(CaveState.FAMILY_HAPPY)
            else:
                self.display_surface = self.family_cage[frame_index]

            self.play(self.hiv_i_reb, "HivIReb", 0.5)
            self.play(self.hugoline_tak, "HugolineTak", 1)

        elif self.state == CaveState.FAMILY_HAPPY:
            self.fadeout_music()
            self.play(self.fanfare, "Fanfare")

            if frame_index >= len(self.happy):
                self.end()
            else:
                self.display_surface = self.happy[frame_index]

        if self.display_surface is not None:
            screen.blit(self.display_surface, (0,0))

        if (self.state == CaveState.SCYLLA_LEAVES or self.state == CaveState.SCYLLA_BIRD or self.state == CaveState.SCYLLA_ROPES) and self.selected_rope is not None:
            pos = self.hugo_sprite_pos[self.selected_rope]
            screen.blit(self.hugo[0], (pos[0], pos[1]))

        if self.rolling_score < self.score:
            self.rolling_score += min(10, self.score - self.rolling_score)
        elif self.sounding_score:
            pygame.mixer.Sound.stop(self.score_counter)
            self.sounding_score = False

        x_score = 243
        y_score = 203
        x_space = 16
        thousands = self.rolling_score // 1000
        hundreds = (self.rolling_score - thousands * 1000) // 100
        tens = (self.rolling_score - thousands * 1000 - hundreds * 100) // 10
        ones = self.rolling_score - thousands * 1000 - hundreds * 100 - tens * 10
        screen.blit(self.score_font[thousands], (x_score + x_space * 0, y_score))
        screen.blit(self.score_font[hundreds], (x_score + x_space * 1, y_score))
        screen.blit(self.score_font[tens], (x_score + x_space * 2, y_score))
        screen.blit(self.score_font[ones], (x_score + x_space * 3, y_score))

    def play_steps(self, frame_index):
        if frame_index % 8 == 0:
            self.play(self.fodtrin1, "Fodtrin1")
            if "Fodtrin2" in self.played:
                del self.played["Fodtrin2"]
        elif frame_index % 8 == 4:
            self.play(self.fodtrin2, "Fodtrin2")
            if "Fodtrin1" in self.played:
                del self.played["Fodtrin1"]

    def calc_win_or_lose(self):
        random_value = random.randint(0, 3)
        if random_value == 0: # LOST
            self.switch_to(self.get_dead_state())
        elif random_value == 1: # WON BIRD
            self.switch_to(CaveState.SCYLLA_BIRD)
            self.score *= 2
        elif random_value == 2:  # WON LEFT
            self.switch_to(CaveState.SCYLLA_LEAVES)
            self.score *= 3
        elif random_value == 3:  # WON ROPE
            self.switch_to(CaveState.SCYLLA_ROPES)
            self.score *= 4

        if self.score != self.rolling_score:
            pygame.mixer.Sound.play(self.score_counter, loops=-1)
            self.sounding_score = True

    def get_dead_state(self):
        if self.selected_rope == 0:
            return CaveState.LOST_FIRST
        elif self.selected_rope == 1:
            return CaveState.LOST_SECOND
        else:
            return CaveState.LOST_THIRD

    def end(self):
        self.fadeout_music()
        pygame.mixer.Sound.stop(self.score_counter)
        pygame.mixer.Sound.stop(self.fanfare)
        self.ended = True

    def fadeout_music(self):
        if self.playing_music:
            if self.argentine_version:
                pygame.mixer.Sound.fadeout(self.bg_music, 1000)
            else:
                pygame.mixer.Sound.fadeout(self.stemning, 1000)
            self.playing_music = False
