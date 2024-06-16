import pygame
import subprocess
import pygame.freetype 
import os
import keyboard
import random
import time
from enum import Enum
from pyvidplayer2 import Video
from PIL import ImageGrab, ImageChops

class GameState(Enum):
    ATTRACT_DEMO = 0
    INITIAL_VIDEO = 1
    PRESS_5_VIDEO = 2
    HAVE_LUCK = 3
    PLAYING_HUGO = 4
    YOU_LOST = 5

def stop_all():
    global vid_a
    global vid_b
    global vid_c
    global vid_d
    global vid_e
    vid_a.stop()
    vid_b.stop()
    vid_c.stop()
    vid_d.stop()
    vid_e.stop()

def write_config(file_path):
    game_options = [
        "Plane",
        "Forest",
        #"Mountain", NO ANDA POR FREEZEFRAME
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

def play_hugo():
    HUGO_DIR = "C:\\Users\\Gzalo\\Desktop\\HugoGoldFlashparty\\"
    HUGO_EXECUTABLE = HUGO_DIR + "hugo.exe"
    HUGO_CONFIG = HUGO_DIR + "Machine.cnf"
    write_config(HUGO_CONFIG)

    return subprocess.Popen(HUGO_EXECUTABLE, cwd = HUGO_DIR)

def capture_screen():
    return ImageGrab.grab()

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

def kill_process(proc):
    try:
        proc.terminate()
        proc.wait(timeout=1)
    except Exception as e:
        print(f"Failed to terminate the process: {e}")

pygame.init()

BTN_OFF_HOOK = "q"
BTN_HUNG_UP = "w"
BTN_END = "e"
BTN_PLAY = "5"

screen = pygame.display.set_mode((480, 360))
pygame.display.set_caption("A jugar con Hugo!")
pygame.font.init()
GAME_FONT = pygame.freetype.SysFont("Arial", 10)
overlay = pygame.image.load("overlay.png").convert()

vid_a = Video("videos/a.mp4")
vid_b = Video("videos/b.mp4")
vid_c = Video("videos/c.mp4")
vid_d = Video("videos/d.mp4")
vid_e = Video("videos/e.mp4")

state = GameState.ATTRACT_DEMO

vid_draw = None
hugo_proc = None
proc_start_time = None
same_image_counter = 0
last_image = capture_screen()
initial_pressed = False

running = True
while running:
    offhook_event = False
    hung_up_event = False
    end_event = False
    play_event = False

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            stop_all()
            running = False

    if keyboard.is_pressed(BTN_OFF_HOOK):
        offhook_event = True
    if keyboard.is_pressed(BTN_HUNG_UP):
        hung_up_event = True
    if keyboard.is_pressed(BTN_PLAY):
        play_event = True
    if keyboard.is_pressed(BTN_END):
        end_event = True

    if hugo_proc != None:
        if hugo_proc.poll() != None:
            end_event = True

        current_image = capture_screen()
        if images_are_equal(last_image, current_image):
            same_image_counter += 1
        else:
            same_image_counter = 0
            last_image = current_image

        if same_image_counter > 50 and initial_pressed == False:
            keyboard.press_and_release('enter')
            initial_pressed = True

        if same_image_counter > 100:
            kill_process(hugo_proc)

    if state == GameState.ATTRACT_DEMO:
        vid_draw = vid_a

        if vid_a.active == False:
            vid_a.restart()
        
        if offhook_event:
            state = GameState.INITIAL_VIDEO
            stop_all()
            vid_b.restart()

        if play_event:
            stop_all()
            state = GameState.PLAYING_HUGO
            hugo_proc = play_hugo()
            proc_start_time = time.time()
            initial_pressed = False

    elif state == GameState.INITIAL_VIDEO:
        vid_draw = vid_b

        if vid_b.active == False:
            state = GameState.PRESS_5_VIDEO
            stop_all()
            vid_c.restart()

        if hung_up_event:
            state = GameState.ATTRACT_DEMO
            stop_all()
            vid_a.restart()

    elif state == GameState.PRESS_5_VIDEO:
        vid_draw = vid_c

        if vid_c.active == False:
            vid_c.restart()

        if play_event:
            state = GameState.HAVE_LUCK
            stop_all()
            vid_e.restart()            

        if hung_up_event:
            state = GameState.ATTRACT_DEMO
            stop_all()
            vid_a.restart()

    elif state == GameState.HAVE_LUCK:
        vid_draw = vid_e

        if vid_e.active == False:
            state = GameState.PLAYING_HUGO
            stop_all()         
            hugo_proc = play_hugo()
            proc_start_time = time.time()
            initial_pressed = False

        if hung_up_event:
            state = GameState.ATTRACT_DEMO
            stop_all()
            vid_a.restart()            

    elif state == GameState.PLAYING_HUGO:
        vid_draw = None

        if hung_up_event:
            state = GameState.ATTRACT_DEMO
            kill_process(hugo_proc)
            stop_all()
            vid_a.restart()

        if end_event:
            state = GameState.YOU_LOST
            stop_all()
            vid_d.restart()            

    elif state == GameState.YOU_LOST:
        vid_draw = vid_d

        if hung_up_event:
            state = GameState.ATTRACT_DEMO
            stop_all()
            vid_a.restart()

        if vid_d.active == False:
            vid_d.restart()

    screen.fill((255,255,255))

    if vid_draw and vid_draw.draw(screen, (0, 0), force_draw=False):
        text_surface, rect = GAME_FONT.render(str(state), (0, 0, 0))
        screen.blit(text_surface, (10, 340))
        screen.blit(overlay, (400, 10))
        pygame.display.update()
    
    pygame.time.wait(16)

pygame.quit()