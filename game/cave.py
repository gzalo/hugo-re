from enum import Enum

import random
import time

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
    state: CaveState = None
    state_start = None
    hugo_sprite_pos = [
        (25, 105),
        (97, 100),
        (172, 102),
    ]
    argentine_version = True

    def __init__(self, score):
        self.talks = Resource.load_surfaces("STAIRS.TIL", 0, 12)
        self.climbs = Resource.load_surfaces("STAIRS.TIL", 11, 51)
        self.firstRope = Resource.load_surfaces("caselive.TIL", 0, 32)
        self.secondRope = Resource.load_surfaces("caselive.TIL", 33, 72)
        self.thirdRope = Resource.load_surfaces("caselive.TIL", 73, 121)
        self.scyllaLeaves = Resource.load_surfaces("caselive.TIL", 122, 177)
        self.scyllaBird = Resource.load_surfaces("caselive.TIL", 178, 240)
        self.scyllaRopes = Resource.load_surfaces("caselive.TIL", 241, 283)
        self.scyllaSpring = Resource.load_surfaces("caselive.TIL", 284, 318)
        self.familyCage = Resource.load_surfaces("caselive.TIL", 319, 352)
        self.hugoPuffFirst = Resource.load_surfaces("casedie.TIL", 122, 166)
        self.hugoPuffSecond = Resource.load_surfaces("casedie.TIL", 167, 211)
        self.hugoPuffThird = Resource.load_surfaces("casedie.TIL", 212, 256)
        self.hugoSpring = Resource.load_surfaces("casedie.TIL", 257, 295)
        self.happy = Resource.load_surfaces("happy.TIL", 0, 111)
        self.Score = Resource.load_surfaces("score.cgf", 0, 9)
        self.Hugo = Resource.load_surfaces("hugo.cgf", 0, 0)

        self.SyncHugoStart = Resource.load_sync("002-06.oos")
        self.SyncHugoDie = Resource.load_sync("002-09.oos")

        self.HerErVi = Resource.load_speak("002-05.wav") # Help us Hugo
        self.TrappeSnak = Resource.load_speak("002-06.wav")  # I feel like this is dangerous
        self.NuKommerJeg = Resource.load_speak("002-07.wav") # Hello is there anyone here
        self.AfskyliaSnak = Resource.load_speak("002-08.wav") # Go ahead and choose
        self.HugoKatapult = Resource.load_speak("002-09.wav") # I'll be back
        self.HugoSkydUd = Resource.load_speak("002-10.wav") # Hugo screams on lost
        self.AfskyliaSkydUd = Resource.load_speak("002-11.wav") # Scylla screams on win
        self.HugolineTak = Resource.load_speak("002-12.wav") # We are free

        self.Stemning = Resource.load_sfx("ba-13.wav") # Background chimes
        self.Fodtrin1 = Resource.load_sfx("ba-15.wav") # Step 1
        self.Fodtrin2 = Resource.load_sfx("ba-16.wav") # Step 2
        self.HivIReb = Resource.load_sfx("ba-17.wav") # Rope used
        self.Fjeder = Resource.load_sfx("ba-18.wav") # Spring
        self.PrePuf = Resource.load_sfx("ba-21.wav") # Anticipation
        self.Puf = Resource.load_sfx("ba-22.wav") # Smoke puff
        self.TastTrykket = Resource.load_sfx("ba-24.wav") # Waiting for you ding
        self.PreFanfare = Resource.load_sfx("ba-101.wav") # Choose something background
        self.Fanfare = Resource.load_sfx("ba-102.wav") # Correct ending (This was starting to ...)
        self.FugleSkrig = Resource.load_sfx("ba-104.wav") # Scylla turned into bird
        self.TrappeGrin = Resource.load_sfx("hexhaha.wav") # Odd laugh
        self.Skrig = Resource.load_sfx("skrig.wav") # Scilla screams
        self.ScoreCounter = Resource.load_sfx("counter.wav") # Score counter increase
        self.BgMusic = Resource.load_sfx("a_party_for_me.mp3") # BG music, downloaded from https://www.youtube.com/watch?v=maJVRkefBhw

        self.played = {}
        self.switch_to(CaveState.WAITING_BEFORE_TALKING)

        self.display_surface = None
        self.ended = False

        self.score = score
        self.rolling_score = self.score

        self.sounding_score = False
        self.selected_rope = None

    def process_events(self, event3, event6, event9, event_plus, event_minus):
        if event_plus:
            self.switch_to(self.state.next())
            self.played = {}
        if event_minus:
            self.switch_to(self.state.previous())
            self.played = {}

        if self.state == CaveState.WAITING_INPUT:
            if event3:
                self.switch_to(CaveState.GOING_FIRST)
                pygame.mixer.Sound.play(self.TastTrykket)
                self.selected_rope = 0
            if event6:
                self.switch_to(CaveState.GOING_SECOND)
                pygame.mixer.Sound.play(self.TastTrykket)
                self.selected_rope = 1
            if event9:
                self.switch_to(CaveState.GOING_THIRD)
                pygame.mixer.Sound.play(self.TastTrykket)
                self.selected_rope = 2

    def switch_to(self, new_state: CaveState):
        self.state = new_state
        self.state_start = time.time()

    def play(self, sound, sound_name, delay: float = 0 ):
        if time.time() - self.state_start > delay and not sound_name in self.played:
            pygame.mixer.Sound.play(sound)
            self.played[sound_name] = True

    def render(self, screen):
        state_time = time.time() - self.state_start
        frame_index = int(state_time * 10)

        if self.state == CaveState.WAITING_BEFORE_TALKING:
            self.play(self.HerErVi, "HerErVi")
            if self.argentine_version:
                self.play(self.BgMusic, "BgMusic")
            else:
                self.play(self.Stemning, "Stemning")

            if state_time > 2.5:
                self.switch_to(CaveState.TALKING_BEFORE_CLIMB)

            self.display_surface = self.talks[12]

        elif self.state == CaveState.TALKING_BEFORE_CLIMB:
            self.play(self.TrappeSnak, "TrappeSnak")
            if not self.argentine_version:
                self.play(self.TrappeGrin, "TrappeGrin", 4)

            if frame_index >= len(self.SyncHugoStart):
                self.switch_to(CaveState.CLIMBING)
            else:
                actual_index = self.SyncHugoStart[frame_index]
                self.display_surface = self.talks[actual_index]

        elif self.state == CaveState.CLIMBING:
            self.play(self.NuKommerJeg, "NuKommerJeg", 1)
            if not self.argentine_version:
                self.play(self.PreFanfare, "PreFanfare", 2)

            if frame_index >= len(self.climbs):
                self.switch_to(CaveState.WAITING_INPUT)
            else:
                self.display_surface = self.climbs[frame_index]

        elif self.state == CaveState.WAITING_INPUT:
            if not self.argentine_version:
                self.play(self.Stemning, "Stemning")
            self.play(self.AfskyliaSnak, "AfskyliaSnak", 0.5)
            self.display_surface = self.firstRope[0]

        elif self.state == CaveState.GOING_FIRST:
            if frame_index >= len(self.firstRope):
                self.calc_win_or_lose()
            else:
                self.display_surface = self.firstRope[frame_index]

            self.play_steps(frame_index)
            self.play(self.HivIReb, "HivIReb", 2)

        elif self.state == CaveState.GOING_SECOND:
            if frame_index >= len(self.secondRope):
                self.calc_win_or_lose()
            else:
                self.display_surface = self.secondRope[frame_index]

            self.play_steps(frame_index)
            self.play(self.HivIReb, "HivIReb", 3)

        elif self.state == CaveState.GOING_THIRD:
            if frame_index >= len(self.thirdRope):
                self.calc_win_or_lose()
            else:
                self.display_surface = self.thirdRope[frame_index]

            self.play_steps(frame_index)
            self.play(self.HivIReb, "HivIReb", 4)

        elif self.state == CaveState.LOST_FIRST:
            if frame_index >= len(self.hugoPuffFirst):
                self.switch_to(CaveState.LOST_SPRING)
            else:
                self.display_surface = self.hugoPuffFirst[frame_index]

            self.play(self.PrePuf, "PrePuf", 1)
            self.play(self.Puf, "Puf", 2)

        elif self.state == CaveState.LOST_SECOND:
            if frame_index >= len(self.hugoPuffSecond):
                self.switch_to(CaveState.LOST_SPRING)
            else:
                self.display_surface = self.hugoPuffSecond[frame_index]

            self.play(self.PrePuf, "PrePuf", 1)
            self.play(self.Puf, "Puf", 2)

        elif self.state == CaveState.LOST_THIRD:
            if frame_index >= len(self.hugoPuffThird):
                self.switch_to(CaveState.LOST_SPRING)
            else:
                self.display_surface = self.hugoPuffThird[frame_index]

            self.play(self.PrePuf, "PrePuf", 1)
            self.play(self.Puf, "Puf", 2)

        elif self.state == CaveState.LOST_SPRING:
            if frame_index >= len(self.hugoSpring):
                self.end()
            else:
                self.display_surface = self.hugoSpring[frame_index]

            self.play(self.HugoKatapult, "HugoKatapult")
            self.play(self.HugoSkydUd, "HugoSkydUd", 2.5)

        elif self.state == CaveState.SCYLLA_LEAVES:
            if frame_index >= len(self.scyllaLeaves):
                self.switch_to(CaveState.FAMILY_CAGE_OPENS)
            else:
                self.display_surface = self.scyllaLeaves[frame_index]

            self.play(self.Skrig, "Skrig", 0.5)

        elif self.state == CaveState.SCYLLA_BIRD:
            if frame_index >= len(self.scyllaBird):
                self.switch_to(CaveState.FAMILY_CAGE_OPENS)
            else:
                self.display_surface = self.scyllaBird[frame_index]

            self.play(self.FugleSkrig, "FugleSkrig", 0.5)

        elif self.state == CaveState.SCYLLA_ROPES:
            if frame_index >= len(self.scyllaRopes):
                self.switch_to(CaveState.SCYLLA_SPRING)
            else:
                self.display_surface = self.scyllaRopes[frame_index]

            self.play(self.PrePuf, "PrePuf",1)
            self.play(self.Puf, "Puf", 2)

        elif self.state == CaveState.SCYLLA_SPRING:
            if frame_index >= len(self.scyllaSpring):
                self.switch_to(CaveState.FAMILY_CAGE_OPENS)
            else:
                self.display_surface = self.scyllaSpring[frame_index]

            self.play(self.Fjeder, "Fjeder", 1.5)
            self.play(self.AfskyliaSkydUd, "AfskyliaSkydUd", 2)

        elif self.state == CaveState.FAMILY_CAGE_OPENS:
            if frame_index >= len(self.familyCage):
                self.switch_to(CaveState.FAMILY_HAPPY)
            else:
                self.display_surface = self.familyCage[frame_index]

            self.play(self.HivIReb, "HivIReb", 0.5)
            self.play(self.HugolineTak, "HugolineTak", 1)

        elif self.state == CaveState.FAMILY_HAPPY:
            if self.bg_music is not None:
                self.bg_music.stop()
            self.play(self.Fanfare, "Fanfare")

            if frame_index >= len(self.happy):
                self.end()
            else:
                self.display_surface = self.happy[frame_index]

        if self.display_surface is not None:
            screen.blit(self.display_surface, (0, 0))

        if (self.state == CaveState.SCYLLA_LEAVES or self.state == CaveState.SCYLLA_BIRD or self.state == CaveState.SCYLLA_ROPES) and self.selected_rope is not None:
            screen.blit(self.Hugo[0], self.hugo_sprite_pos[self.selected_rope])

        if self.rolling_score < self.score:
            self.rolling_score += min(10, self.score - self.rolling_score)
        elif self.sounding_score:
            pygame.mixer.Sound.stop(self.ScoreCounter)
            self.sounding_score = False

        x_score = 243
        y_score = 203
        x_space = 16
        thousands = self.rolling_score // 1000
        hundreds = (self.rolling_score - thousands * 1000) // 100
        tens = (self.rolling_score - thousands * 1000 - hundreds * 100) // 10
        ones = self.rolling_score - thousands * 1000 - hundreds * 100 - tens * 10
        screen.blit(self.Score[thousands], (x_score + x_space * 0, y_score))
        screen.blit(self.Score[hundreds], (x_score + x_space * 1, y_score))
        screen.blit(self.Score[tens], (x_score + x_space * 2, y_score))
        screen.blit(self.Score[ones], (x_score + x_space * 3, y_score))

    def play_steps(self, frame_index):
        if frame_index % 8 == 0:
            self.play(self.Fodtrin1, "Fodtrin1")
            if "Fodtrin2" in self.played:
                del self.played["Fodtrin2"]
        elif frame_index % 8 == 4:
            self.play(self.Fodtrin2, "Fodtrin2")
            if "Fodtrin1" in self.played:
                del self.played["Fodtrin1"]

    def calc_win_or_lose(self):
        random_value = random.randint(0, 3)
        if random_value == 0: # LOST
            self.switch_to(CaveState.LOST_FIRST if self.selected_rope == 0 else CaveState.LOST_SECOND if self.selected_rope == 1 else CaveState.LOST_THIRD)
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
            pygame.mixer.Sound.play(self.ScoreCounter, loops=-1)
            self.sounding_score = True

    def end(self):
        if self.argentine_version:
            pygame.mixer.Sound.fadeout(self.BgMusic, 1000)
        else:
            pygame.mixer.Sound.fadeout(self.Stemning, 1000)

        pygame.mixer.Sound.stop(self.ScoreCounter)

        self.ended = True