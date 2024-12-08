import pygame

from forest.forest_resources import ForestResources
from forest.playing import Playing
from state import State


class TalkingAfterHurt(State):
    def process_events(self, events):
        if (self.parent.lives == 1 and self.get_frame_index() >= len(ForestResources.sync_lastlife)) or \
            (self.parent.lives >= 1 and self.get_frame_index() >= len(ForestResources.sync_dieonce)):
            return Playing(self.parent)

        if self.one_shot(0.5, "knock1"):
            pygame.mixer.Sound.play(ForestResources.sfx_hugo_knock)

        if self.one_shot(0, "speak"):
            pygame.mixer.Sound.play(ForestResources.sfx_hugo_knock)

            if self.parent.lives == 1:
                pygame.mixer.Sound.play(ForestResources.speak_lastlife)
            else:
                pygame.mixer.Sound.play(ForestResources.speak_dieonce)

    def render(self, screen):
        self.parent.render_background(screen)
        self.parent.render_bottom(screen)

        sync = ForestResources.sync_lastlife if self.parent.lives == 1 else ForestResources.sync_dieonce
        screen.blit(ForestResources.hugo_telllives[sync[self.get_frame_index()]-1], (128, -16))

        if self.get_frame_index() < 8 and self.get_frame_index() % 4 == 0:
            screen.blit(ForestResources.hugo_hand2[0], (96, 78))
        else:
            screen.blit(ForestResources.hugo_hand1[0], (96, 83))

