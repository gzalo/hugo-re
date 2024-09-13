import pygame


class Config:
    TITLE = "Hugo - Into the Multiverse"
    SCR_WIDTH = 640
    SCR_HEIGHT = 480
    SCR_FULLSCREEN = False

    EFFECT_DURATION_FLASH = 2
    EFFECT_DURATION_INVERT = 5
    INSTRUCTIONS_TIMEOUT = 3
    ARGENTINE_VERSION = True

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

    COUNTRIES = ["ar", "cl", "dn", "fr"]

    GAMES = {
        "Forest": {
            "name": "Selva",
        }
    }

    FOREST_BG_SPEED_MULTIPLIER = 1.0
    FOREST_GROUND_SPEED = 75
    FOREST_MAX_TIME = 60