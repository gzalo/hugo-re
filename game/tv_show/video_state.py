from audio_helper import AudioHelper
from phone_events import PhoneEvents
from state import State


class VideoState(State):
    def __init__(self, context, video, loop = False, audio_resource = None):
        super().__init__(context)
        self.video = video
        self.loop = loop
        self.has_looped = False
        self.audio_resource = audio_resource
        self.audio_instance_id = None

    def on_enter(self) -> None:
        super().on_enter()
        self.video.restart()
        if self.audio_resource:
            self.audio_instance_id = AudioHelper.play(self.audio_resource, self.context.audio_port, loops=-1)

    def process_events(self, phone_events: PhoneEvents):
        if self.loop and not self.video.active:
            self.video.restart()
            self.has_looped = True
            # Restart audio if it's not already playing
            if self.audio_resource and self.audio_instance_id is None:
                self.audio_instance_id = AudioHelper.play(self.audio_resource, self.context.audio_port, loops=-1)

    def render(self, screen):
        self.video.draw(screen, (0, 0))

    def on_exit(self) -> None:
        super().on_exit()
        self.video.stop()
        if self.audio_instance_id is not None:
            AudioHelper.stop(self.audio_instance_id, self.context.audio_port)
            self.audio_instance_id = None

    def video_ended(self):
        return not self.video.active