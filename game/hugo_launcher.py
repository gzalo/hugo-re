import random
import subprocess
import time

import win32gui
import keyboard
import win32con
from PIL import ImageGrab, ImageChops
import easyocr
import numpy


def write_config(file_path, game):
    new_line = f"AutoStart = {game};"

    with open(file_path, 'r') as file:
        lines = file.readlines()

    if len(lines) >= 2:
        lines[1] = new_line + '\n'
    else:
        lines.append(new_line + '\n')

    with open(file_path, 'w') as file:
        file.writelines(lines)


def images_are_equal(img1, img2):
    return ImageChops.difference(img1, img2).getbbox() is None


class HugoLauncher:
    hugo_proc = None
    same_image_counter = 0
    last_image = ImageGrab.grab()
    initial_pressed = False
    title = None
    current_game = "Forest"
    reader = easyocr.Reader(['en'])
    last_read_time = None
    score = 0

    game_options = [
        "Plane",
        "Forest",
        # "Mountain", NO ANDA POR FREEZEFRAME
        # "LumberJack", CRASH UNKNOWN
        "IceCavern",
        # "Labyrinth", TOO COMPLEX
        "SkateBoard",
        "Scuba",
        "Train"
    ]

    # left, top, right, bottom
    score_rect_per_game = {
        "Plane": (327,10,460,64),
        "Forest": (380,388,579,461),
        "IceCavern": (445,0,639,67),
        "SkateBoard": (479,17,614,70),
        "Scuba": (0,0,195,70),
        "Train": (367,17,505,71)
    }

    def __init__(self, title):
        self.last_read_time = time.time()
        self.title = title

    def start(self):
        hugo_dir = "C:\\Users\\Gzalo\\Desktop\\HugoGoldFlashparty\\"
        hugo_exe = hugo_dir + "hugo.exe"
        hugo_config = hugo_dir + "Machine.cnf"
        write_config(hugo_config, self.current_game)
        self.score = 0

        handle = win32gui.FindWindow(0, self.title)
        win32gui.ShowWindow(handle, win32con.SW_MINIMIZE)
        self.hugo_proc = subprocess.Popen(hugo_exe, cwd=hugo_dir)

    def end(self):
        if self.hugo_proc is None:
            return
        try:
            self.hugo_proc.terminate()
            self.hugo_proc.wait(timeout=1)
        except Exception as e:
            print(f"Failed to terminate the process: {e}")

    def process(self):
        if self.hugo_proc is not None:
            if self.hugo_proc.poll() is not None:
                self.hugo_proc = None
                handle = win32gui.FindWindow(0, self.title)
                win32gui.ShowWindow(handle, win32con.SW_RESTORE)
                return True

            current_image = ImageGrab.grab()
            if images_are_equal(self.last_image, current_image):
                self.same_image_counter += 1
            else:
                self.same_image_counter = 0
                self.last_image = current_image

            if self.same_image_counter > 50 and self.initial_pressed is False:
                keyboard.press_and_release('enter')
                self.initial_pressed = True

            if self.same_image_counter > 100:
                self.end()

            if time.time() - self.last_read_time > 2:
                score_image = ImageGrab.grab(bbox=self.score_rect_per_game[self.current_game])
                results = self.reader.readtext(numpy.array(score_image), allowlist='0123456789')
                for result in results:
                    if result[2] > 0.65:
                        score = int(result[1])
                        if self.score < score < 9999:
                            self.score = score
                    else:
                        print(result[2])

                self.last_read_time = time.time()

        return False

    def set_random_game(self):
        new_game = random.choice(self.game_options)
        while new_game == self.current_game:
            new_game = random.choice(self.game_options)
        self.current_game = new_game

    def get_game(self):
        return self.current_game

    def get_games(self):
        return self.game_options

    def get_score(self):
        return self.score
