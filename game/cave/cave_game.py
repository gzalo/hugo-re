from audio_helper import AudioHelper
from cave.cave_resources import CaveResources
from cave.waiting_before_talking import WaitingBeforeTalking
from game_data import GameData
import global_state
from null_state import NullState
from phone_events import PhoneEvents
import pygame

class CaveGame:

    def __init__(self, context: GameData):
        self.ended = False
        self.rolling_score = context.forest_score
        self.context = context

        self.sounding_score = False
        self.cave_score_counter_start = None
        self._state = WaitingBeforeTalking(context)
        self._state.on_enter()

    def process_events(self, phone_events: PhoneEvents):
        next_state = self._state.process_events(phone_events)

        if next_state is not None:
            self._state.on_exit()
            if next_state == NullState:
                self.end()
            else:
                self._state = next_state(self.context)
                self._state.on_enter()

        if self.context.forest_score != self.rolling_score and not self.sounding_score:
            self.context.cave_score_counter_id = AudioHelper.play(CaveResources.score_counter, self.context.audio_port)
            self.cave_score_counter_start = global_state.frame_time
            self.sounding_score = True
        elif self.sounding_score and global_state.frame_time - self.cave_score_counter_start >= 0.1:
            self.context.cave_score_counter_id = AudioHelper.play(CaveResources.score_counter, self.context.audio_port)
            self.cave_score_counter_start = global_state.frame_time

    def render(self, screen):
        self._state.render(screen)

        if self.rolling_score < self.context.forest_score:
            self.rolling_score += min(10, self.context.forest_score - self.rolling_score)
        elif self.sounding_score:
            AudioHelper.stop(self.context.cave_score_counter_id, self.context.audio_port)
            self.sounding_score = False

        self.render_score(screen)

    def render_score(self, screen):
        x_score_right = 210 + 16 * 6  # Right edge position (6 digits max)
        y_score = 203
        x_space = 16

        score = int(self.rolling_score)
        if score == 0:
            screen.blit(CaveResources.score_font[0], (x_score_right - x_space, y_score))
            return

        digits = []
        temp = score
        while temp > 0:
            digits.append(temp % 10)
            temp //= 10

        for i, digit in enumerate(digits):
            screen.blit(CaveResources.score_font[digit], (x_score_right - x_space * (i + 1), y_score))

    def end(self):
        self._state.on_exit()
        if self.sounding_score:
            AudioHelper.stop(self.context.cave_score_counter_id, self.context.audio_port)
            self.sounding_score = False
        self.ended = True
