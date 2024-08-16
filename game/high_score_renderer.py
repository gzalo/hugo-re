import time
import random
import pygame

class HighScoreRenderer:

    def __init__(self, scores, game_options, game_names):
        self.scores = scores
        self.score_font = pygame.freetype.SysFont("Arial", 28, bold=True)
        self.score_game = game_options[0]
        self.game_options = game_options
        self.game_names = game_names
        self.time_score = time.time()

    def set_random_score_game(self):
        if len(self.game_options) == 1:
            self.score_game = self.game_options[0]
            return
        new_game = random.choice(self.game_options)
        while new_game == self.score_game:
            new_game = random.choice(self.game_options)
        self.score_game = new_game

    def render_outline(self, screen, text, xpos, ypos):
        text_surface_bg, _ = self.score_font.render(text, (0, 0, 0))
        text_surface_fg, _ = self.score_font.render(text, (255, 255, 255))
        screen.blit(text_surface_bg, (xpos - 1, ypos - 1))
        screen.blit(text_surface_bg, (xpos + 1, ypos - 1))
        screen.blit(text_surface_bg, (xpos - 1, ypos + 1))
        screen.blit(text_surface_bg, (xpos + 1, ypos + 1))
        screen.blit(text_surface_bg, (xpos - 2, ypos - 2))
        screen.blit(text_surface_bg, (xpos + 2, ypos - 2))
        screen.blit(text_surface_bg, (xpos - 2, ypos + 2))
        screen.blit(text_surface_bg, (xpos + 2, ypos + 2))
        screen.blit(text_surface_fg, (xpos, ypos))

    def render(self, screen):
        if time.time() - self.time_score > 4:
            self.set_random_score_game()
            self.time_score = time.time()

        self.render_outline(screen, "PUNTAJES " + self.game_names[self.score_game], 30, 30)

        top_scores = self.scores.get_top_scores(self.score_game, 5)
        for rank, (name, score) in enumerate(top_scores, start=1):
            self.render_outline(screen, f"{rank}. {name} - {score}", 30, 30 + rank * 30)
