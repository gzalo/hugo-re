from enum import Enum

class GameState(Enum):
    ATTRACT = 0
    INITIAL = 1
    PRESS_5 = 2
    HAVE_LUCK = 3
    PLAYING_HUGO = 4
    YOU_LOST = 5