import pygame
import pygame.freetype
import keyboard
import time
from pyvidplayer2 import Video
import speech_recognition as sr

from hugo_launcher import HugoLauncher
from game_state import GameState

class Game:
    BTN_OFF_HOOK = "q"
    BTN_HUNG_UP = "w"
    BTN_END = "e"
    BTN_PLAY = "5"
    BTN_NEXT_GAME = "1"
    BTN_EXIT = "r"
    TITLE = "A jugar con Hugo!"
    INSTRUCTIONS_TIMEOUT = 5
    SCR_WIDTH = 640
    SCR_HEIGHT = 480

    state = GameState.ATTRACT
    state_start = time.time()
    hugo_launcher = HugoLauncher(TITLE)
    user_name = "TEST"
    name_font = None

    videos = {
        GameState.ATTRACT: Video("videos/attract_demo.mp4"),
        GameState.INITIAL: Video("videos/hello_hello.mp4"),
        GameState.YOUR_NAME: Video("videos/your_name_is.mp4"),
        GameState.NICE_NAME: Video("videos/nice_name.mp4"),
        GameState.PRESS_5: Video("videos/press_5.mp4"),
        GameState.YOU_LOST: Video("videos/you_lost.mp4"),
        GameState.GOING_SCYLLA: Video("videos/scylla_cave.mp4"),
        GameState.HAVE_LUCK: Video("videos/have_luck.mp4"),
    }

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

    def hasEnded(self):
        return self.state in self.videos and not self.videos[self.state].active

    def state_timeout(self, timeout):
        return time.time() - self.state_start > timeout

    def reset_state_timeout(self):
        self.state_start = time.time()

    def run(self):
        pygame.init()

        screen = pygame.display.set_mode((self.SCR_WIDTH, self.SCR_HEIGHT), pygame.FULLSCREEN)
        pygame.mouse.set_visible(False)
        pygame.display.set_caption(self.TITLE)
        pygame.font.init()
        debug_font = pygame.freetype.SysFont("Arial", 8)
        self.name_font = pygame.freetype.SysFont("Arial", 45)
        overlay = pygame.image.load("overlay.png").convert()

        instructions = {game_name:pygame.image.load("instructions/" + game_name + ".png").convert() for game_name in self.hugo_launcher.get_games()}
        prev_next_game_event = False

        running = True
        while running:
            offhook_event = False
            hung_up_event = False
            end_proc_event = False
            press_5_event = False
            next_game_event = False

            for event in pygame.event.get():
                if event.type == pygame.QUIT or keyboard.is_pressed(self.BTN_EXIT):
                    self.switch_to(None)
                    running = False

            if keyboard.is_pressed(self.BTN_OFF_HOOK):
                offhook_event = True
            if keyboard.is_pressed(self.BTN_HUNG_UP):
                hung_up_event = True
            if keyboard.is_pressed(self.BTN_PLAY):
                press_5_event = True
            if keyboard.is_pressed(self.BTN_END):
                end_proc_event = True

            if keyboard.is_pressed(self.BTN_NEXT_GAME):
                next_game_event = True

            if self.hugo_launcher.process():
                end_proc_event = True

            if self.state != GameState.ATTRACT:
                if hung_up_event:
                    self.switch_to(GameState.ATTRACT)
                    self.hugo_launcher.end()

            if self.state == GameState.ATTRACT:
                self.reloop()

                if offhook_event:
                    self.switch_to(GameState.INITIAL)

                if press_5_event:
                    self.switch_to(GameState.HAVE_LUCK)

            elif self.state == GameState.INITIAL:
                if self.hasEnded():
                    self.switch_to(GameState.YOUR_NAME)
                    self.user_name = ""

            elif self.state == GameState.YOUR_NAME:
                if self.hasEnded():

                    r = sr.Recognizer()
                    with sr.AudioFile("hola_gonzalo.wav") as source:
                        audio = r.record(source)

                    try:
                        words = r.recognize_whisper(audio, language="es")
                        self.user_name = words.split(" ")[-1]
                    except sr.UnknownValueError:
                        print("Sphinx could not understand audio")
                    except sr.RequestError as e:
                        print("Sphinx error; {0}".format(e))

                if press_5_event:
                    self.switch_to(GameState.NICE_NAME)

            elif self.state == GameState.NICE_NAME:
                if self.hasEnded():
                    self.switch_to(GameState.PRESS_5)

            elif self.state == GameState.PRESS_5:
                self.reloop()

                if press_5_event:
                    self.switch_to(GameState.HAVE_LUCK)

            elif self.state == GameState.HAVE_LUCK:
                if self.hasEnded():
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
                pass

            elif self.state == GameState.PLAYING_SCYLLA:
                if end_proc_event:
                    self.switch_to(GameState.YOU_LOST)

            elif self.state == GameState.YOU_LOST:
                pass

            vid_draw = self.videos[self.state] if self.state in self.videos else None
            if vid_draw and vid_draw.draw(screen, (0, 0), force_draw=False):
                text_surface, rect = debug_font.render(str(self.state), (0, 0, 0))
                screen.blit(text_surface, (10, 460))
                screen.blit(overlay, (520, 15))

            if self.state == GameState.INSTRUCTIONS:
                screen.blit(instructions[self.hugo_launcher.get_game()], (0,0))

            if self.state == GameState.YOUR_NAME:
                self.render_name(screen)

            pygame.display.update()
            pygame.time.wait(16)
            prev_next_game_event = next_game_event

        pygame.quit()

    def render_name(self, screen):
        text_surface_bg, rect = self.name_font.render(self.user_name, (0, 0, 0))
        text_surface_fg, rect = self.name_font.render(self.user_name, (255, 255, 255))
        xpos = self.SCR_WIDTH / 2 - rect.width / 2
        ypos = self.SCR_HEIGHT / 2 - rect.height / 2
        screen.blit(text_surface_bg, (xpos - 1, ypos - 1))
        screen.blit(text_surface_bg, (xpos + 1, ypos - 1))
        screen.blit(text_surface_bg, (xpos - 1, ypos + 1))
        screen.blit(text_surface_bg, (xpos + 1, ypos + 1))
        screen.blit(text_surface_fg, (xpos, ypos))


if __name__ == "__main__":
    Game().run()
