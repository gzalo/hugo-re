import pygame
import pygame.freetype
import moderngl
import time
from array import array

from scores import Scores
from tv_show import TvShow
from phone_events import PhoneEvents

class Game:
    BTN_OFF_HOOK = [pygame.K_F1, pygame.K_F3, pygame.K_F5, pygame.K_F7]
    BTN_HUNG_UP = [pygame.K_F2, pygame.K_F4, pygame.K_F6, pygame.K_F8]
    BTN_EXIT = pygame.K_F12

    BTN_1 = [pygame.K_1, pygame.K_4, pygame.K_7, pygame.K_KP1]
    BTN_2 = [pygame.K_2, pygame.K_5, pygame.K_8, pygame.K_KP2]
    BTN_3 = [pygame.K_3, pygame.K_6, pygame.K_9, pygame.K_KP3]
    BTN_4 = [pygame.K_q, pygame.K_r, pygame.K_u, pygame.K_KP4]
    BTN_5 = [pygame.K_w, pygame.K_t, pygame.K_i, pygame.K_KP5]
    BTN_6 = [pygame.K_e, pygame.K_y, pygame.K_o, pygame.K_KP6]
    BTN_7 = [pygame.K_a, pygame.K_f, pygame.K_j, pygame.K_KP7]
    BTN_8 = [pygame.K_s, pygame.K_g, pygame.K_k, pygame.K_KP8]
    BTN_9 = [pygame.K_d, pygame.K_h, pygame.K_l, pygame.K_KP9]
    BTN_0 = [pygame.K_z, pygame.K_x, pygame.K_c, pygame.K_KP0]

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

    phone_positions = [
        (0, 0),
        (320, 0),
        (0, 240),
        (320, 240),
    ]

    tv_shows = [
        TvShow(games, "ar"),
        TvShow(games, "cl"),
        TvShow(games, "dn"),
        TvShow(games, "fr"),
    ]

    scores = Scores()
    start_time = time.time()
    waviness = 0

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
        phone_icons = [pygame.image.load("images/phone" + str(phone_index) + "_small.png").convert_alpha() for phone_index in range(4)]
        phone_icons_active = [pygame.image.load("images/phone" + str(phone_index) + "_small_active.png").convert_alpha() for phone_index in range(4)]

        running = True
        while running:
            phone_events = [PhoneEvents() for _ in range(4)]

            for event in pygame.event.get():
                if event.type == pygame.QUIT or (event.type == pygame.KEYDOWN and event.key == self.BTN_EXIT):
                    running = False

                if event.type == pygame.KEYDOWN:
                    for i in range(4):
                        if event.key == self.BTN_OFF_HOOK[i]:
                            phone_events[i].offhook = True
                        if event.key == self.BTN_HUNG_UP[i]:
                            phone_events[i].hungup = True
                        if event.key == self.BTN_0[i]:
                            phone_events[i].press_0 = True
                        if event.key == self.BTN_1[i]:
                            phone_events[i].press_1 = True
                        if event.key == self.BTN_2[i]:
                            phone_events[i].press_2 = True
                        if event.key == self.BTN_3[i]:
                            phone_events[i].press_3 = True
                        if event.key == self.BTN_4[i]:
                            phone_events[i].press_4 = True
                        if event.key == self.BTN_5[i]:
                            phone_events[i].press_5 = True
                        if event.key == self.BTN_6[i]:
                            phone_events[i].press_6 = True
                        if event.key == self.BTN_7[i]:
                            phone_events[i].press_7 = True
                        if event.key == self.BTN_8[i]:
                            phone_events[i].press_8 = True
                        if event.key == self.BTN_9[i]:
                            phone_events[i].press_9 = True

            any_playing = False
            for tv_show in self.tv_shows:
                tv_show.handle_events(phone_events[self.tv_shows.index(tv_show)])
                tv_show.render(self.display, self.positions[self.tv_shows.index(tv_show)], instructions)
                if tv_show.is_playing():
                    any_playing = True

            if not any_playing:
                self.display.blit(logo, (0, 0))
            else:
                for i in range(4):
                    if phone_events[i].any_set():
                        self.display.blit(phone_icons_active[i], self.phone_positions[i])
                    else:
                        self.display.blit(phone_icons[i], self.phone_positions[i])

            self.render_frame(not any_playing)
            pygame.time.wait(16)

        pygame.quit()

    def surf_to_texture(self, surf):
        tex = self.ctx.texture(surf.get_size(), 4)
        tex.filter = (moderngl.LINEAR, moderngl.LINEAR)
        tex.swizzle = 'BGRA'
        tex.write(surf.get_view('1'))
        return tex

    def render_frame(self, wavy):
        frame_tex = self.surf_to_texture(self.display)
        frame_tex.use(0)
        self.program['tex'] = 0
        self.program['time'] = time.time() - self.start_time
        self.program['wavyness'] = 0.01 if wavy else 0
        self.render_object.render(mode=moderngl.TRIANGLE_STRIP)
        pygame.display.flip()
        frame_tex.release()

if __name__ == "__main__":
    Game().run()
