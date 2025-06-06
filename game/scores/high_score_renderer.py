import random
import pygame
import pygame.freetype

import global_state

class HighScoreRenderer:

    def __init__(self, scores, games):
        self.scores = scores
        self.score_font = pygame.freetype.SysFont("Arial", 28, bold=True)
        self.score_game = next(iter(games))
        self.games = games
        self.time_score = global_state.frame_time

    def set_random_score_game(self):
        if len(self.games) == 1:
            self.score_game = next(iter(self.games))
            return
        new_game = random.choice(list(self.games.keys()))
        while new_game == self.score_game:
            new_game = random.choice(list(self.games.keys()))
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
        if global_state.frame_time - self.time_score > 4:
            self.set_random_score_game()
            self.time_score = global_state.frame_time

        self.render_outline(screen, "PUNTAJES " + self.games[self.score_game]["name"], 30, 30)

        top_scores = self.scores.get_top_scores(self.score_game, 5)
        for rank, (name, score) in enumerate(top_scores, start=1):
            self.render_outline(screen, f"{rank}. {name} - {score}", 30, 30 + rank * 30)
