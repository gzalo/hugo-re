from enum import Enum

class GameState(Enum):
    ATTRACT = 0
    INITIAL = 1
    YOUR_NAME = 2
    NICE_NAME = 4
    PRESS_5 = 5
    HAVE_LUCK = 6
    INSTRUCTIONS = 7
    PLAYING_HUGO = 8
    GOING_SCYLLA = 9
    PLAYING_SCYLLA = 10
    YOU_LOST = 11