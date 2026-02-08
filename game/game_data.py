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
    forest_controls_inverted: bool

    # Scoreboard tracking
    forest_normal_sacks_collected: int = 0
    forest_golden_sacks_collected: int = 0
    forest_reached_end: bool = False
    forest_obstacles_hit: list = None  # List of obstacle types that caused life loss

    cave_selected_rope: int = 0
    cave_win_type: int = 0

    forest_bg_atmosphere_id: int = None
    cave_score_counter_id: int = None
    cave_fanfare_id: int = None
    cave_bg_music_id: int = None
    cave_stemning_id: int = None
    scoreboard_score_counter_id: int = None
