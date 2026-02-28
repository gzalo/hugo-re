import sys
import pygame

import converter


class Resource:
    if len(sys.argv) > 1:
        DATA_DIR = sys.argv[1]
    else:
        raise Exception("Data directory needs to be specified as first argument")

    @staticmethod
    def load_sync(game, filename):
        path = Resource.DATA_DIR + "/" + game + "/Syncs/" + filename
        return converter.decode_oos(path)

    @staticmethod
    def load_speak(game, filename):
        path = "/speak/" if game == "RopeOutroData" else "/speaks/"
        resource_path = game + path + filename
        # Return resource path string instead of pygame.mixer.Sound
        return resource_path

    @staticmethod
    def load_sfx(game, filename):
        if game == "RopeOutroData":
            sfx = "SFX"
        else:
            sfx = "sfx"

        resource_path = game + "/" + sfx + "/" + filename
        # Return resource path string instead of pygame.mixer.Sound
        return resource_path

    @staticmethod
    def load_surfaces(game, name, start, end):
        if game == "RopeOutroData":
            gfx = "GFX"
        else:
            gfx = "gfx"

        file_path = Resource.DATA_DIR + "/" + game + "/" + gfx + "/" + name
        all_frames = converter.decode_surfaces(file_path)
        return all_frames[start:end + 1]

    @staticmethod
    def load_surface_raw(game, name):
        filename = Resource.DATA_DIR + "/" + game + "/gfx/" + name
        return pygame.image.load(filename).convert_alpha()

    @staticmethod
    def load_surface_res(name):
        filename = "resources/" + name
        return pygame.image.load(filename).convert_alpha()
