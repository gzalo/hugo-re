import json
import pygame
import sys


class Resource:
    if len(sys.argv) > 1:
        DATA_DIR = sys.argv[1]
    else:
        raise Exception("Data directory needs to be specified as first argument")

    resources = {}

    @staticmethod
    def load_sync(game, filename):
        with open(Resource.DATA_DIR + "/" + game + "/syncs/" + filename + ".json") as f:
            return json.load(f)["frameIndices"]

    @staticmethod
    def load_speak(game, filename):
        cache_key = f"speak_{game}_{filename}"
        if cache_key in Resource.resources:
            return Resource.resources[cache_key]
        else:
            path = "/speak/" if game == "RopeOutroData" else "/speaks/"
            sfx = pygame.mixer.Sound(Resource.DATA_DIR + "/" + game + path + filename)
            Resource.resources[cache_key] = sfx
            return sfx

    @staticmethod
    def load_sfx(game, filename):
        cache_key = f"sfx_{game}_{filename}"
        if cache_key in Resource.resources:
            return Resource.resources[cache_key]
        else:
            sfx = pygame.mixer.Sound(Resource.DATA_DIR + "/" + game + "/sfx/" + filename)
            Resource.resources[cache_key] = sfx
            return sfx

    @staticmethod
    def load_surfaces(game, name, start, end):
        out = []

        for frame in range(start, end + 1):
            cache_key = f"surface_{game}_{name}_{frame}"

            if cache_key in Resource.resources:
                out.append(Resource.resources[cache_key])
            else:
                filename = Resource.DATA_DIR + "/" + game + "/gfx/" + name + "_" + str(frame) + ".png"
                surf = pygame.image.load(filename).convert_alpha()
                Resource.resources[cache_key] = surf
                out.append(surf)

        return out

    @staticmethod
    def load_surface_raw(game, name):
        cache_key = f"surfaceraw_{game}_{name}"

        if cache_key in Resource.resources:
            return Resource.resources[cache_key]
        else:
            filename = Resource.DATA_DIR + "/" + game + "/gfx/" + name
            surf = pygame.image.load(filename).convert_alpha()
            Resource.resources[cache_key] = surf
            return surf
