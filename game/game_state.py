from enum import Enum

class GameState(Enum):
    ATTRACT = 0
    INITIAL = 1
    YOUR_NAME = 2
    NICE_NAME = 3
    PRESS_5 = 4
    HAVE_LUCK = 5
    INSTRUCTIONS = 6
    PLAYING_HUGO = 7
    GOING_SCYLLA = 8
    PLAYING_SCYLLA = 9
    YOU_LOST = 10