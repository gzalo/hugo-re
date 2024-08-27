from phone_events import PhoneEvents
from state import State


class VideoState(State):
    def __init__(self, parent, video, loop = False):
        super().__init__(parent)
        self.video = video
        self.loop = loop

    def on_enter(self) -> None:
        super().on_enter()
        self.video.restart()

    def process_events(self, phone_events: PhoneEvents):
        if self.loop and not self.video.active:
            self.video.restart()

    def render(self, screen):
        self.video.draw(screen, (0, 0))

    def on_exit(self) -> None:
        super().on_exit()
        self.video.stop()

    def video_ended(self):
        return not self.video.active