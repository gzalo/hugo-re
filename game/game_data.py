from dataclasses import dataclass


@dataclass
class GameData:
    country: str
    audio_port: int

    forest_score: int
    forest_lives: int
    forest_parallax_pos: int
    forest_sacks: list
    forest_obstacles: list
    forest_leaves: list

    cave_selected_rope: int
    cave_win_type: int
