from cave.cave_resources import CaveResources
from cave.waiting_before_talking import WaitingBeforeTalking
from null_state import NullState
from phone_events import PhoneEvents
import pygame

class CaveGame:

    def __init__(self, context):
        self.ended = False
        self.rolling_score = context.forest_score
        self.context = context
        context.cave_selected_rope = None

        self.sounding_score = False
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

        if self.context.forest_score != self.rolling_score:
            pygame.mixer.Sound.play(CaveResources.score_counter, loops=-1)
            self.sounding_score = True

    def render(self, screen):
        self._state.render(screen)

        if self.rolling_score < self.context.forest_score:
            self.rolling_score += min(10, self.context.forest_score - self.rolling_score)
        elif self.sounding_score:
            pygame.mixer.Sound.stop(CaveResources.score_counter)
            self.sounding_score = False

        self.render_score(screen)

    def render_score(self, screen):
        x_score = 243
        y_score = 203
        x_space = 16
        thousands = self.rolling_score // 1000
        hundreds = (self.rolling_score - thousands * 1000) // 100
        tens = (self.rolling_score - thousands * 1000 - hundreds * 100) // 10
        ones = self.rolling_score - thousands * 1000 - hundreds * 100 - tens * 10
        screen.blit(CaveResources.score_font[thousands], (x_score + x_space * 0, y_score))
        screen.blit(CaveResources.score_font[hundreds], (x_score + x_space * 1, y_score))
        screen.blit(CaveResources.score_font[tens], (x_score + x_space * 2, y_score))
        screen.blit(CaveResources.score_font[ones], (x_score + x_space * 3, y_score))

    def end(self):
        self._state.on_exit()
        pygame.mixer.Sound.stop(CaveResources.score_counter)
        self.ended = True
