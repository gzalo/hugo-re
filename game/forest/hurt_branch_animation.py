from forest.forest_resources import ForestResources
from forest.hurt_branch_talking import HurtBranchTalking
from state import State


class HurtBranchAnimation(State):
    def process_events(self, events):
        if self.get_frame_index() >= len(ForestResources.hugohitlog):
            return HurtBranchTalking(self.parent)

    def render(self, screen):
        screen.blit(ForestResources.hugohitlog[self.get_frame_index()], (0,0))
