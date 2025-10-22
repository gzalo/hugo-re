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
    forest_bg_atmosphere_id: int = None

    cave_selected_rope: int
    cave_win_type: int
    cave_score_counter_id: int = None
    cave_fanfare_id: int = None
    cave_bg_music_id: int = None
    cave_stemning_id: int = None
