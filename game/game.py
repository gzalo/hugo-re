import os
import random
import platform

import pygame
import pygame.freetype
import moderngl
import time
from array import array

from cave.cave_resources import CaveResources
from config import Config
from effects.splat import Splat
from forest.forest_resources import ForestResources
from game_state import GameData
from post_processing import PostProcessing
from scores.scores import Scores
from tv_show.tv_show_parent import TvShowParent
from phone_events import PhoneEvents
from tv_show.tv_show_resources import TvShowResources
from tween import Tween
import global_state

class Game:
    positions = [
        (0, 0),
        (320, 0),
        (0, 240),
        (320, 240),
    ]

    scores = Scores()
    start_time = time.time()
    waviness = 0
    tv_shows = []
    post_processing = None
    pos_by_country = {}
    effective_attacks = []

    with open("resources/shaders/main.vert", "r") as f:
        vert_shader = f.read()

    with open("resources/shaders/main.frag", "r") as f:
        frag_shader = f.read()

    if platform.system() == "Darwin":
        pygame.display.gl_set_attribute(pygame.GL_CONTEXT_PROFILE_MASK, pygame.GL_CONTEXT_PROFILE_CORE)
        vert_shader = vert_shader.replace("#version 300 es", "#version 330 core")
        frag_shader = frag_shader.replace("#version 300 es", "#version 330 core")
                
    def run(self):
        pygame.init()

        fs = pygame.FULLSCREEN if Config.SCR_FULLSCREEN or "FULLSCREEN" in os.environ else 0
        fs |= pygame.OPENGL | pygame.DOUBLEBUF
        pygame.display.set_mode((Config.SCR_WIDTH, Config.SCR_HEIGHT), fs)
        display = pygame.Surface((Config.SCR_WIDTH, Config.SCR_HEIGHT))
        ctx = moderngl.create_context(310)

        quad_buffer = ctx.buffer(data=array('f', [
            -1.0, 1.0, 0.0, 0.0,
            1.0, 1.0, 1.0, 0.0,
            -1.0, -1.0, 0.0, 1.0,
            1.0, -1.0, 1.0, 1.0,
        ]))

        program = ctx.program(vertex_shader=self.vert_shader, fragment_shader=self.frag_shader)
        render_object = ctx.vertex_array(program, [(quad_buffer, '2f 2f', 'vert', 'texcoord')])
        self.post_processing = PostProcessing()

        loading = pygame.image.load("resources/images/loading.png").convert_alpha()
        display.blit(loading, (0, 0))
        global_state.frame_time = time.time()
        self.render_frame(ctx, display, program, render_object, False)

        pygame.mouse.set_visible(False)
        pygame.display.set_caption(Config.TITLE)
        pygame.font.init()

        logo = pygame.image.load("resources/images/logo.png").convert_alpha()
        phone_icons = [pygame.image.load("resources/images/phone" + str(phone_index) + "_small.png").convert_alpha() for phone_index in range(4)]
        phone_icons_active = [pygame.image.load("resources/images/phone" + str(phone_index) + "_small_active.png").convert_alpha() for phone_index in range(4)]
        screens = [pygame.Surface((320, 240)) for _ in range(4)]

        CaveResources.init()
        ForestResources.init()
        TvShowResources.init()
        Splat.init()

        self.tv_shows = [TvShowParent(GameData(country, 0, 0, 0, [], [], [], 0, 0)) for country in Config.COUNTRIES]
        self.pos_by_country = {tv_show.country: self.positions[idx] for idx, tv_show in enumerate(self.tv_shows)}

        clock = pygame.time.Clock()

        running = True
        while running:
            phone_events = [PhoneEvents() for _ in range(4)]

            for event in pygame.event.get():
                if event.type == pygame.QUIT or (event.type == pygame.KEYDOWN and event.key == Config.BTN_EXIT) or (event.type == pygame.MOUSEBUTTONDOWN and event.button == 1):
                    running = False

                if event.type == pygame.KEYDOWN:
                    for i in range(4):
                        if event.key == Config.BTN_OFF_HOOK[i]:
                            phone_events[i].offhook = True
                        if event.key == Config.BTN_HUNG_UP[i]:
                            phone_events[i].hungup = True
                        if event.key == Config.BTN_0[i]:
                            phone_events[i].press_0 = True
                        if event.key == Config.BTN_1[i]:
                            phone_events[i].press_1 = True
                        if event.key == Config.BTN_2[i]:
                            phone_events[i].press_2 = True
                        if event.key == Config.BTN_3[i]:
                            phone_events[i].press_3 = True
                        if event.key == Config.BTN_4[i]:
                            phone_events[i].press_4 = True
                        if event.key == Config.BTN_5[i]:
                            phone_events[i].press_5 = True
                        if event.key == Config.BTN_6[i]:
                            phone_events[i].press_6 = True
                        if event.key == Config.BTN_7[i]:
                            phone_events[i].press_7 = True
                        if event.key == Config.BTN_8[i]:
                            phone_events[i].press_8 = True
                        if event.key == Config.BTN_9[i]:
                            phone_events[i].press_9 = True

            any_playing = False
            pre_render = time.time()

            global_state.frame_time = time.time()

            for tv_show in self.tv_shows:
                index = self.tv_shows.index(tv_show)
                tv_show.handle_events(phone_events[index])
                tv_show.render(screens[index])
                if tv_show.is_playing():
                    any_playing = True

            post_render = time.time()

            self.post_processing.handle_events()

            for attack in global_state.attacks:
                global_state.attacks.remove(attack)
                current_country = attack[0]
                effect = attack[1]
                start_time = attack[2]
                valid_shows = [tv_show for tv_show in self.tv_shows if tv_show.country != current_country and tv_show.is_playing()]
                if len(valid_shows) == 0:
                    continue
                random_player = random.choice(valid_shows)
                random_player.external_effect(effect)
                self.effective_attacks.append((current_country, random_player.country, start_time))

            for i in range(4):
                display.blit(screens[i], self.positions[i])

            if not any_playing:
                display.blit(logo, (0,0))
            else:
                for i in range(4):
                    if phone_events[i].any_set():
                        display.blit(phone_icons_active[i], self.positions[i])
                    else:
                        display.blit(phone_icons[i], self.positions[i])

                # Render orbs
                if self.effective_attacks:
                    curr_time = time.time()

                    for attack in self.effective_attacks:
                        pos_0 = self.pos_by_country[attack[0]]
                        pos_1 = self.pos_by_country[attack[1]]
                        dt = curr_time - attack[2]
                        orb_x = Tween.map_ease_in(dt, 0, Config.EFFECT_DURATION_ORB, pos_0[0]+70, pos_1[0]+70)
                        orb_y = Tween.map_ease_in(dt, 0, Config.EFFECT_DURATION_ORB, pos_0[1]+70, pos_1[1]+70)
                        display.blit(Splat.orb, (orb_x, orb_y))
                        if dt > Config.EFFECT_DURATION_ORB:
                            self.effective_attacks.remove(attack)

            self.render_frame(ctx, display, program, render_object, any_playing)
            post_shader = time.time()

            render_time = (post_render - pre_render) * 1000
            shader_time = (post_shader - post_render) * 1000
            caption = f"Render: {render_time:.2f} ms, Shader: {shader_time:.2f} ms"
            pygame.display.set_caption(caption)

            clock.tick(30)

        pygame.quit()

    @staticmethod
    def surf_to_texture(ctx, surf):
        tex = ctx.texture(surf.get_size(), 4)
        tex.filter = (moderngl.LINEAR, moderngl.LINEAR)
        tex.swizzle = 'BGRA'
        tex.write(surf.get_view('1'))
        return tex

    def render_frame(self, ctx, display, program, render_object, any_playing):
        frame_tex = self.surf_to_texture(ctx, display)
        frame_tex.use(0)
        program['tex'] = 0
        program['time'] = global_state.frame_time - self.start_time
        self.post_processing.apply(program, any_playing)
        render_object.render(mode=moderngl.TRIANGLE_STRIP)
        pygame.display.flip()
        frame_tex.release()

if __name__ == "__main__":
    Game().run()
