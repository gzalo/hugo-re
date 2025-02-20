import json
import pygame
import sys


class Resource:
    if len(sys.argv) > 1:
        DATA_DIR = sys.argv[1]
    else:
        raise Exception("Data directory needs to be specified as first argument")

    @staticmethod
    def load_sync(game, filename):
        with open(Resource.DATA_DIR + "/" + game + "/Syncs/" + filename + ".json") as f:
            return json.load(f)["frameIndices"]

    @staticmethod
    def load_speak(game, filename):
        path = "/speak/" if game == "RopeOutroData" else "/speaks/"
        return pygame.mixer.Sound(Resource.DATA_DIR + "/" + game + path + filename)

    @staticmethod
    def load_sfx(game, filename):
        if game == "RopeOutroData":
            sfx = "SFX"
        else:
            sfx = "sfx"

        return pygame.mixer.Sound(Resource.DATA_DIR + "/" + game + "/" + sfx + "/" + filename)

    @staticmethod
    def load_surfaces(game, name, start, end):
        out = []

        if game == "RopeOutroData":
            gfx = "GFX"
        else:
            gfx = "gfx"

        for frame in range(start, end + 1):
            filename = Resource.DATA_DIR + "/" + game + "/" + gfx + "/" + name + "_" + str(frame) + ".png"
            out.append(pygame.image.load(filename).convert_alpha())

        return out

    @staticmethod
    def load_surface_raw(game, name):
        filename = Resource.DATA_DIR + "/" + game + "/gfx/" + name
        return pygame.image.load(filename).convert_alpha()
