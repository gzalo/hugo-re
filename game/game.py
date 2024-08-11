import pygame
import pygame.freetype
import time
from pyvidplayer2 import Video
import random

from hugo_launcher import HugoLauncher
from game_state import GameState
from scores import Scores


class Game:
    BTN_OFF_HOOK = pygame.K_q
    BTN_HUNG_UP = pygame.K_w
    BTN_END = pygame.K_e
    BTN_PLAY = pygame.K_5
    BTN_NEXT_GAME = pygame.K_6
    BTN_UP = pygame.K_8
    BTN_DOWN = pygame.K_2
    BTN_EXIT = pygame.K_r
    TITLE = "A jugar con Hugo!"
    INSTRUCTIONS_TIMEOUT = 5
    SCR_WIDTH = 640
    SCR_HEIGHT = 480
    SCR_FULLSCREEN = False

    state = GameState.ATTRACT
    state_start = time.time()
    hugo_launcher = HugoLauncher()
    scores = Scores()
    user_name = ""
    name_font = None
    score_font = None
    time_score = time.time()
    score_game = "Forest"

    videos = {
        GameState.ATTRACT: Video("videos/attract_demo.mp4"),
        GameState.INITIAL: Video("videos/hello_hello.mp4"),
        GameState.YOUR_NAME: Video("videos/your_name_is.mp4"),
        GameState.NICE_NAME: Video("videos/nice_name.mp4"),
        GameState.PRESS_5: Video("videos/press_5.mp4"),
        GameState.ENDING: Video("videos/you_lost.mp4"),
        GameState.GOING_SCYLLA: Video("videos/scylla_cave.mp4"),
        GameState.PLAYING_SCYLLA_0: Video("rope/0.mp4"),
        GameState.PLAYING_SCYLLA_1_BAD: Video("rope/1_bad.mp4"),
        GameState.PLAYING_SCYLLA_1_GOOD: Video("rope/1_good.mp4"),
        GameState.PLAYING_SCYLLA_1_BEST: Video("rope/1_best.mp4"),
        GameState.PLAYING_SCYLLA_2_BAD: Video("rope/2_bad.mp4"),
        GameState.PLAYING_SCYLLA_2_GOOD: Video("rope/2_good.mp4"),
        GameState.PLAYING_SCYLLA_2_BEST: Video("rope/2_best.mp4"),
        GameState.PLAYING_SCYLLA_3_BAD: Video("rope/3_bad.mp4"),
        GameState.PLAYING_SCYLLA_3_GOOD: Video("rope/3_good.mp4"),
        GameState.PLAYING_SCYLLA_3_BEST: Video("rope/3_best.mp4"),
        GameState.HAVE_LUCK: Video("videos/have_luck.mp4"),
    }

    game_names = {
        "Plane": "Avión",
        "Forest": "Selva",
        "IceCavern": "Cueva de hielo",
        "SkateBoard": "Skate",
        "Scuba": "Buceo",
        "Train": "Tren"
    }

    user_name_len = 3

    def set_random_score_game(self):
        new_game = random.choice(self.hugo_launcher.game_options)
        while new_game == self.score_game:
            new_game = random.choice(self.hugo_launcher.game_options)
        self.score_game = new_game

    def switch_to(self, new_state: GameState | None):
        for videoKey, video in self.videos.items():
            video.stop()
        if new_state is not None and new_state in self.videos:
            self.videos[new_state].restart()
        self.state = new_state
        self.state_start = time.time()

    def reloop(self):
        if self.state in self.videos and not self.videos[self.state].active:
            self.videos[self.state].restart()

    def has_ended(self):
        return self.state in self.videos and not self.videos[self.state].active

    def state_timeout(self, timeout):
        return time.time() - self.state_start > timeout

    def reset_state_timeout(self):
        self.state_start = time.time()

    def run(self):
        pygame.init()

        fs = pygame.FULLSCREEN if self.SCR_FULLSCREEN else 0
        screen = pygame.display.set_mode((self.SCR_WIDTH, self.SCR_HEIGHT), fs)
        pygame.mouse.set_visible(False)
        pygame.display.set_caption(self.TITLE)
        pygame.font.init()
        debug_font = pygame.freetype.SysFont("Arial", 8)
        self.name_font = pygame.freetype.SysFont("Arial", 45)
        self.score_font = pygame.freetype.SysFont("Arial", 28, bold=True)
        self.big_score_font = pygame.freetype.SysFont("Arial", 55, bold=True)
        overlay = pygame.image.load("images/overlay.png").convert()
        name_ask = pygame.image.load("images/name_ask.png").convert()
        keyboard_surface = [
            pygame.image.load("images/keyboard_0.png").convert_alpha(),
            pygame.image.load("images/keyboard_1.png").convert_alpha(),
            pygame.image.load("images/keyboard_2.png").convert_alpha()
        ]

        instructions = {game_name:pygame.image.load("instructions/" + game_name + ".png").convert() for game_name in self.hugo_launcher.get_games()}
        prev_next_game_event = False
        prev_up_event = False
        prev_down_event = False

        running = True
        updated_score = False
        while running:
            offhook_event = False
            hung_up_event = False
            end_proc_event = False
            press_5_event = False
            next_game_event = False
            up_event = False
            down_event = False
            press_1_event = False
            press_2_event = False
            press_3_event = False

            for event in pygame.event.get():
                if event.type == pygame.QUIT or (event.type == pygame.KEYDOWN and event.key == self.BTN_EXIT):
                    self.switch_to(None)
                    running = False


            keys = pygame.key.get_pressed()

            if keys[self.BTN_OFF_HOOK]:
                offhook_event = True
            if keys[self.BTN_HUNG_UP]:
                hung_up_event = True
            if keys[self.BTN_PLAY]:
                press_5_event = True
            if keys[self.BTN_END]:
                end_proc_event = True
            if keys[self.BTN_NEXT_GAME]:
                next_game_event = True
            if keys[self.BTN_UP]:
                up_event = True
            if keys[self.BTN_DOWN]:
                down_event = True
            if keys[pygame.K_1]:
                press_1_event = True
            if keys[pygame.K_2]:
                press_2_event = True
            if keys[pygame.K_3]:
                press_3_event = True

            if self.hugo_launcher.process():
                end_proc_event = True

            if self.state != GameState.ATTRACT:
                if hung_up_event:
                    self.switch_to(GameState.ATTRACT)
                    self.hugo_launcher.end()
                    if self.user_name != "":
                        self.scores.insert_score(self.hugo_launcher.current_game, self.user_name, self.hugo_launcher.score)

            if self.state == GameState.ATTRACT:
                self.reloop()

                if offhook_event:
                    self.switch_to(GameState.INITIAL)

                if press_5_event:
                    self.switch_to(GameState.PRESS_5)

            elif self.state == GameState.INITIAL:
                if self.has_ended():
                    self.switch_to(GameState.YOUR_NAME)
                    self.user_name = ""

            elif self.state == GameState.YOUR_NAME:
                self.reloop()

                if self.user_name == "":
                    self.user_name = "A"

                if up_event and not prev_up_event:
                    last_char = ord(self.user_name[-1])
                    self.user_name = self.user_name[:-1] + chr((last_char - 65 + 1) % 26 + 65)

                if down_event and not prev_down_event:
                    last_char = ord(self.user_name[-1])
                    self.user_name = self.user_name[:-1] + chr((last_char - 65 - 1) % 26 + 65)

                if next_game_event and not prev_next_game_event:
                    if len(self.user_name) == self.user_name_len:
                        self.switch_to(GameState.NICE_NAME)
                    else:
                        self.user_name += "A"

                if press_5_event:
                    self.switch_to(GameState.NICE_NAME)

            elif self.state == GameState.NICE_NAME:
                if self.has_ended():
                    self.switch_to(GameState.PRESS_5)

            elif self.state == GameState.PRESS_5:
                self.reloop()

                if press_5_event:
                    self.switch_to(GameState.HAVE_LUCK)

            elif self.state == GameState.HAVE_LUCK:
                if self.has_ended():
                    self.switch_to(GameState.INSTRUCTIONS)
                    self.hugo_launcher.set_random_game()

            elif self.state == GameState.INSTRUCTIONS:
                if next_game_event and not prev_next_game_event:
                    self.hugo_launcher.set_random_game()
                    self.reset_state_timeout()

                if press_5_event or self.state_timeout(self.INSTRUCTIONS_TIMEOUT):
                    self.switch_to(GameState.PLAYING_HUGO)
                    self.hugo_launcher.start()

            elif self.state == GameState.PLAYING_HUGO:
                if end_proc_event:
                    self.switch_to(GameState.GOING_SCYLLA)

            elif self.state == GameState.GOING_SCYLLA:
                if self.has_ended():
                    self.switch_to(GameState.PLAYING_SCYLLA_0)

            elif self.state == GameState.PLAYING_SCYLLA_0:
                updated_score = False
                random_value = random.randint(0, 2)
                if press_1_event:
                    if random_value == 0:
                        self.switch_to(GameState.PLAYING_SCYLLA_1_BAD)
                    elif random_value == 1:
                        self.switch_to(GameState.PLAYING_SCYLLA_1_GOOD)
                    else:
                        self.switch_to(GameState.PLAYING_SCYLLA_1_BEST)
                if press_2_event:
                    if random_value == 0:
                        self.switch_to(GameState.PLAYING_SCYLLA_2_BAD)
                    elif random_value == 1:
                        self.switch_to(GameState.PLAYING_SCYLLA_2_GOOD)
                    else:
                        self.switch_to(GameState.PLAYING_SCYLLA_2_BEST)
                if press_3_event:
                    if random_value == 0:
                        self.switch_to(GameState.PLAYING_SCYLLA_3_BAD)
                    elif random_value == 1:
                        self.switch_to(GameState.PLAYING_SCYLLA_3_GOOD)
                    else:
                        self.switch_to(GameState.PLAYING_SCYLLA_3_BEST)

            elif self.is_cave() :
                if time.time() - self.state_start > 3 and not updated_score:
                    if self.is_good():
                        self.hugo_launcher.score *= 2
                    elif self.is_best():
                        self.hugo_launcher.score *= 3
                    self.scores.insert_score(self.hugo_launcher.current_game, self.user_name, self.hugo_launcher.score)
                    self.user_name = ""
                    updated_score = True

                if self.has_ended():
                    self.switch_to(GameState.ENDING)

            elif self.state == GameState.ENDING:
                pass

            screen.fill((255,255,255))

            vid_draw = self.videos[self.state] if self.state in self.videos else None
            if vid_draw and vid_draw.draw(screen, (0, 0), force_draw=False):
                text_surface, rect = debug_font.render(str(self.state), (0, 0, 0))
                screen.blit(text_surface, (10, 460))
                if self.state == GameState.ATTRACT:

                    if time.time() - self.time_score > 4:
                        self.set_random_score_game()
                        self.time_score = time.time()

                    self.render_highscores(screen, self.score_game)
                elif self.state == GameState.YOUR_NAME:
                    screen.blit(keyboard_surface[len(self.user_name) - 1], (0, 0))
                    self.render_name(screen)
                elif self.state == GameState.PLAYING_SCYLLA_0 or self.is_cave():
                    self.render_score(screen, str(self.hugo_launcher.score))
                else:
                    screen.blit(overlay, (520, 15))
                pygame.display.update()

            elif self.state == GameState.INSTRUCTIONS:
                screen.blit(instructions[self.hugo_launcher.get_game()], (0, 0))
                pygame.display.update()

            pygame.time.wait(16)
            prev_next_game_event = next_game_event
            prev_up_event = up_event
            prev_down_event = down_event

        pygame.quit()

    def render_name(self, screen):
        for i in range(len(self.user_name)):
            text_surface_bg, rect = self.name_font.render(self.user_name[i], (0, 0, 0))
            text_surface_fg, rect = self.name_font.render(self.user_name[i], (255, 255, 255))
            xpos = 106 + i * 56 - rect.width/2
            ypos = 202
            screen.blit(text_surface_bg, (xpos - 1, ypos - 1))
            screen.blit(text_surface_bg, (xpos + 1, ypos - 1))
            screen.blit(text_surface_bg, (xpos - 1, ypos + 1))
            screen.blit(text_surface_bg, (xpos + 1, ypos + 1))
            screen.blit(text_surface_bg, (xpos - 2, ypos - 2))
            screen.blit(text_surface_bg, (xpos + 2, ypos - 2))
            screen.blit(text_surface_bg, (xpos - 2, ypos + 2))
            screen.blit(text_surface_bg, (xpos + 2, ypos + 2))
            screen.blit(text_surface_fg, (xpos, ypos))

    def render_outline(self, screen, text, xpos, ypos):
        text_surface_bg, rect = self.score_font.render(text, (0, 0, 0))
        text_surface_fg, rect = self.score_font.render(text, (255, 255, 255))
        screen.blit(text_surface_bg, (xpos - 1, ypos - 1))
        screen.blit(text_surface_bg, (xpos + 1, ypos - 1))
        screen.blit(text_surface_bg, (xpos - 1, ypos + 1))
        screen.blit(text_surface_bg, (xpos + 1, ypos + 1))
        screen.blit(text_surface_bg, (xpos - 2, ypos - 2))
        screen.blit(text_surface_bg, (xpos + 2, ypos - 2))
        screen.blit(text_surface_bg, (xpos - 2, ypos + 2))
        screen.blit(text_surface_bg, (xpos + 2, ypos + 2))
        screen.blit(text_surface_fg, (xpos, ypos))

    def is_cave(self):
        return self.state == GameState.PLAYING_SCYLLA_1_BAD or \
             self.state == GameState.PLAYING_SCYLLA_1_GOOD or \
             self.state == GameState.PLAYING_SCYLLA_1_BEST or \
             self.state == GameState.PLAYING_SCYLLA_2_BAD or \
             self.state == GameState.PLAYING_SCYLLA_2_GOOD or \
             self.state == GameState.PLAYING_SCYLLA_2_BEST or \
             self.state == GameState.PLAYING_SCYLLA_3_BAD or \
             self.state == GameState.PLAYING_SCYLLA_3_GOOD or \
             self.state == GameState.PLAYING_SCYLLA_3_BEST

    def render_highscores(self, screen, game_name):
        self.render_outline(screen, "PUNTAJES " + self.game_names[game_name], 30, 30)

        top_scores = self.scores.get_top_scores(game_name, 5)
        for rank, (name, score) in enumerate(top_scores, start=1):
            self.render_outline(screen, f"{rank}. {name} - {score}", 30, 30 + rank * 30)

    def render_score(self, screen, score):
        pygame.draw.rect(screen, (0, 0, 0), (453, 403, 163, 55))
        text_surface_bg, rect = self.big_score_font.render(score, (0, 0, 0))
        text_surface_fg, rect = self.big_score_font.render(score, (255, 255, 255))
        xpos = 600 - rect.width
        ypos = 410
        screen.blit(text_surface_bg, (xpos - 1, ypos - 1))
        screen.blit(text_surface_bg, (xpos + 1, ypos - 1))
        screen.blit(text_surface_bg, (xpos - 1, ypos + 1))
        screen.blit(text_surface_bg, (xpos + 1, ypos + 1))
        screen.blit(text_surface_bg, (xpos - 2, ypos - 2))
        screen.blit(text_surface_bg, (xpos + 2, ypos - 2))
        screen.blit(text_surface_bg, (xpos - 2, ypos + 2))
        screen.blit(text_surface_bg, (xpos + 2, ypos + 2))
        screen.blit(text_surface_fg, (xpos, ypos))

    def is_good(self):
          return self.state == GameState.PLAYING_SCYLLA_1_GOOD or \
             self.state == GameState.PLAYING_SCYLLA_2_GOOD or \
             self.state == GameState.PLAYING_SCYLLA_3_GOOD

    def is_best(self):
          return self.state == GameState.PLAYING_SCYLLA_1_BEST or \
             self.state == GameState.PLAYING_SCYLLA_2_BEST or \
             self.state == GameState.PLAYING_SCYLLA_3_BEST

if __name__ == "__main__":
    Game().run()
