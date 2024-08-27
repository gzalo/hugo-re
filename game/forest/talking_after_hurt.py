import pygame

from forest.forest_resources import ForestResources
from forest.playing import Playing
from state import State


class TalkingAfterHurt(State):
    def process_events(self, events):
        if (self.parent.lifes == 1 and self.get_frame_index() >= len(ForestResources.sync_lastlife)) or \
            (self.parent.lifes >= 1 and self.get_frame_index() >= len(ForestResources.sync_dieonce)):
            return Playing(self.parent)

        if self.one_shot(0.25, "knock1"):
            pygame.mixer.Sound.play(ForestResources.sfx_hugo_knock)

        if self.one_shot(0.5, "knock1"):
            pygame.mixer.Sound.play(ForestResources.sfx_hugo_knock)

        if self.one_shot(0, "speak"):
            pygame.mixer.Sound.play(ForestResources.sfx_hugo_knock)

            if self.parent.lifes == 1:
                pygame.mixer.Sound.play(ForestResources.speak_lastlife)
            else:
                pygame.mixer.Sound.play(ForestResources.speak_dieonce)

    def render(self, screen):
        self.parent.render_background(screen)
        self.parent.render_bottom(screen)

        if self.get_frame_index() < 8:
            if self.get_frame_index() % 2 == 0:
                screen.blit(ForestResources.hugo_hand1[0], (96, 83))
            else:
                screen.blit(ForestResources.hugo_hand2[0], (96, 78))

        sync = ForestResources.sync_lastlife if self.parent.lifes == 1 else ForestResources.sync_dieonce
        screen.blit(ForestResources.hugo_telllives[sync[self.get_frame_index()]], (128, -16))

