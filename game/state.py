from typing import Optional, Type

import global_state
from game_data import GameData
from phone_events import PhoneEvents

class State:
    def __init__(self, context: GameData):
        self.start_time = None
        self.events = {}
        self.context = context

    def process_events(self, phone_events: PhoneEvents) -> Optional[Type['State']]:
        # Do nothing
        pass

    def render(self, screen):
        # Do nothing
        pass

    def on_enter(self) -> None:
        self.start_time = global_state.frame_time

    def on_exit(self) -> None:
        # Do nothing
        pass

    def get_state_time(self):
        return global_state.frame_time - self.start_time

    def get_frame_index(self) -> int:
        return int(self.get_state_time() * 10)

    def get_frame_index_fast(self) -> int:
        return int(self.get_state_time() * 20)

    def one_shot(self, delta: float, name: str) -> bool:
        if name not in self.events and global_state.frame_time - self.start_time > delta:
            self.events[name] = global_state.frame_time - self.start_time
            return True
        return False

    def every(self, delta: float, name: str, offset: float = 0.0) -> bool:
        if name not in self.events:
            if global_state.frame_time - self.start_time > offset:
                self.events[name] = global_state.frame_time + delta
                return True
        elif global_state.frame_time - self.events[name] >= 0:
            self.events[name] = global_state.frame_time + delta
            return True
        return False
