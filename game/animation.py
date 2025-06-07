class Animation:
    @staticmethod
    def get_sync_frame(animations, syncs, frame_index):
        if frame_index >= len(syncs):
            frame_index = len(syncs) - 1

        frame = syncs[frame_index] - 1
        if frame < 0:
            print("Animation index less than 0!")
            frame = 0

        if frame >= len(animations):
            print("Animation index out of range!")
            frame = len(animations) - 1

        return animations[frame]

    @staticmethod
    def get_frame(animations, frame_index):
        if frame_index >= len(animations):
            print("Animation index out of range!")
            frame_index = len(animations) - 1

        return animations[frame_index]


