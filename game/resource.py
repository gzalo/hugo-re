import json
import pygame


class Resource:
    DATA_DIR = "C:/Users/Gzalo/Desktop/HugoGoldFlashparty/BigFileConverted/RopeOutroData"

    @staticmethod
    def load_sync(filename):
        with open(Resource.DATA_DIR + "/syncs/" + filename + ".json") as f:
            return json.load(f)["frameIndices"]

    @staticmethod
    def load_speak(filename):
        return pygame.mixer.Sound(Resource.DATA_DIR + "/speak/" + filename)

    @staticmethod
    def load_sfx(filename):
        return pygame.mixer.Sound(Resource.DATA_DIR + "/sfx/" + filename)

    @staticmethod
    def load_surfaces(name, start, end):
        out = []

        for frame in range(start, end + 1):
            filename = Resource.DATA_DIR + "/gfx/" + name + "_" + str(frame) + ".png"
            out.append(pygame.transform.scale_by(pygame.image.load(filename).convert_alpha(), 2))

        return out