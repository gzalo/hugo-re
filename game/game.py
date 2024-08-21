import pygame
import pygame.freetype
import moderngl
import time
from array import array

from scores import Scores
from tv_show import TvShow
from phone_events import PhoneEvents

class Game:
    BTN_OFF_HOOK = pygame.K_F1
    BTN_HUNG_UP = pygame.K_F2
    BTN_EXIT = pygame.K_F4

    BTN_PLAY = pygame.K_5
    BTN_NEXT_GAME = pygame.K_6
    BTN_UP = pygame.K_8
    BTN_DOWN = pygame.K_2

    TITLE = "Hugo - Into the Multiverse"
    INSTRUCTIONS_TIMEOUT = 5
    SCR_WIDTH = 640
    SCR_HEIGHT = 480
    SCR_FULLSCREEN = False

    games = {
        "Forest": {
            "name": "Selva",
        }
    }

    positions = [
        (0, 0),
        (320, 0),
        (0, 240),
        (320, 240),
    ]

    tv_shows = [
        TvShow(games, "ar"),
        TvShow(games, "ar"),
        TvShow(games, "ar"),
        TvShow(games, "ar"),
    ]

    scores = Scores()
    start_time = time.time()

    with open("shaders/main.vert", "r") as f:
        vert_shader = f.read()

    with open("shaders/main.frag", "r") as f:
        frag_shader = f.read()

    def run(self):
        pygame.init()

        fs = pygame.FULLSCREEN if self.SCR_FULLSCREEN else 0
        fs |= pygame.OPENGL | pygame.DOUBLEBUF
        pygame.display.set_mode((self.SCR_WIDTH, self.SCR_HEIGHT), fs)
        self.display = pygame.Surface((self.SCR_WIDTH, self.SCR_HEIGHT))
        self.ctx = moderngl.create_context()

        quad_buffer = self.ctx.buffer(data=array('f', [
            -1.0, 1.0, 0.0, 0.0,
            1.0, 1.0, 1.0, 0.0,
            -1.0, -1.0, 0.0, 1.0,
            1.0, -1.0, 1.0, 1.0,
        ]))

        self.program = self.ctx.program(vertex_shader=self.vert_shader, fragment_shader=self.frag_shader)
        self.render_object = self.ctx.vertex_array(self.program, [(quad_buffer, '2f 2f', 'vert', 'texcoord')])

        pygame.mouse.set_visible(False)
        pygame.display.set_caption(self.TITLE)
        pygame.font.init()

        logo = pygame.image.load("images/logo.png").convert_alpha()
        instructions = {game_name:pygame.image.load("instructions/" + game_name + ".png").convert() for game_name in self.games.keys()}

        running = True
        while running:
            phone_events = [PhoneEvents() for _ in range(4)]

            for event in pygame.event.get():
                if event.type == pygame.QUIT or (event.type == pygame.KEYDOWN and event.key == self.BTN_EXIT):
                    running = False

                if event.type == pygame.KEYDOWN:
                    if event.key == self.BTN_OFF_HOOK:
                        phone_events[0].offhook = True
                    if event.key == self.BTN_HUNG_UP:
                        phone_events[0].hungup = True
                    if event.key == self.BTN_PLAY:
                        phone_events[0].press_5 = True
                    if event.key == self.BTN_NEXT_GAME:
                        phone_events[0].press_6 = True
                    if event.key == self.BTN_UP:
                        phone_events[0].press_2 = True
                    if event.key == self.BTN_DOWN:
                        phone_events[0].press_8 = True
                    if event.key == pygame.K_3:
                        phone_events[0].press_3 = True
                    if event.key == pygame.K_6:
                        phone_events[0].press_6 = True
                    if event.key == pygame.K_9:
                        phone_events[0].press_9 = True


            self.display.fill((255,255,255))

            for tv_show in self.tv_shows:
                tv_show.handle_events(phone_events[self.tv_shows.index(tv_show)])
                tv_show.render(self.display, self.positions[self.tv_shows.index(tv_show)], instructions)

            self.display.blit(logo, (0, 0))
            self.render_frame()
            pygame.time.wait(16)

        pygame.quit()

    def surf_to_texture(self, surf):
        tex = self.ctx.texture(surf.get_size(), 4)
        tex.filter = (moderngl.LINEAR, moderngl.LINEAR)
        tex.swizzle = 'BGRA'
        tex.write(surf.get_view('1'))
        return tex

    def render_frame(self):
        frame_tex = self.surf_to_texture(self.display)
        frame_tex.use(0)
        self.program['tex'] = 0
        self.program['time'] = time.time() - self.start_time
        self.render_object.render(mode=moderngl.TRIANGLE_STRIP)
        pygame.display.flip()
        frame_tex.release()

if __name__ == "__main__":
    Game().run()
