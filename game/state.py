import time

from phone_events import PhoneEvents


class State:
    def __init__(self, parent):
        self.start_time = None
        self.events = {}
        self.parent = parent

    def process_events(self, phone_events: PhoneEvents):
        # Do nothing
        pass

    def render(self, screen):
        # Do nothing
        pass

    def on_enter(self) -> None:
        self.start_time = time.time()

    def on_exit(self) -> None:
        # Do nothing
        pass

    def get_state_time(self):
        return time.time() - self.start_time

    def get_frame_index(self) -> int:
        return int(self.get_state_time() * 10)

    def get_frame_index_fast(self) -> int:
        return int(self.get_state_time() * 20)

    def one_shot(self, delta: float, name: str) -> bool:
        if name not in self.events and time.time() - self.start_time > delta:
            self.events[name] = time.time() - self.start_time
            return True
        return False

    def every(self, delta: float, name: str, offset: float = 0.0) -> bool:
        if name not in self.events:
            if time.time() - self.start_time > offset:
                self.events[name] = time.time() + delta
                return True
        elif time.time() - self.events[name] >= 0:
            self.events[name] = time.time() + delta
            return True
        return False
