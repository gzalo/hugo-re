import json
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
