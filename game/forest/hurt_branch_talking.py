from forest.forest_resources import ForestResources
from state import State


class HurtBranchTalking(State):
    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.sync_hitlog):
            return self.parent.reduce_lifes()

    def render(self, screen):
        screen.blit(ForestResources.hugohitlog[ForestResources.sync_hitlog[self.get_frame_index()]], (0, 0))