import time
import random
import subprocess
import win32gui
import keyboard
import win32con
from PIL import ImageGrab, ImageChops


def write_config(file_path):
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

    new_line = f"AutoStart = {random.choice(game_options)};"

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


def is_screen_black():
    screen = ImageGrab.grab()
    screen_np = screen.load()
    for x in range(screen.width):
        for y in range(screen.height):
            if screen_np[x, y] != (0, 0, 0):
                return False
    return True


class HugoLauncher:
    proc_start_time = None
    hugo_proc = None
    same_image_counter = 0
    last_image = ImageGrab.grab()
    initial_pressed = False
    title = None

    def __init__(self, title):
        self.title = title

    def start(self):
        hugo_dir = "C:\\Users\\Gzalo\\Desktop\\HugoGoldFlashparty\\"
        hugo_exe = hugo_dir + "hugo.exe"
        hugo_config = hugo_dir + "Machine.cnf"
        write_config(hugo_config)

        self.proc_start_time = time.time()
        self.hugo_proc = subprocess.Popen(hugo_exe, cwd=hugo_dir)

    def end(self):
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

        return False
