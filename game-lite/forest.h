#ifndef FOREST_H
#define FOREST_H

GameState process_forest(InputState state);
void render_forest();
void on_enter_forest();
void render_forest_background();
void render_forest_bottom();
void external_effect(int effect_type);
void end_forest();

// Effect types (matches Python EffectType enum)
typedef enum {
    EFFECT_NONE = 0,
    EFFECT_SPLAT = 1,
    EFFECT_INVERT = 2
} EffectType;

typedef enum {
    STATE_FOREST_NONE,
    STATE_FOREST_BRANCH_ANIMATION,
    STATE_FOREST_BRANCH_TALKING,
    STATE_FOREST_FLYING_FALLING,
    STATE_FOREST_FLYING_FALLING_HANG_ANIMATION,
    STATE_FOREST_FLYING_FALLING_HANG_TALKING,
    STATE_FOREST_FLYING_START,
    STATE_FOREST_FLYING_TALKING,
    STATE_FOREST_ROCK_ANIMATION,
    STATE_FOREST_ROCK_HIT_ANIMATION,
    STATE_FOREST_ROCK_TALKING,
    STATE_FOREST_TRAP_ANIMATION,
    STATE_FOREST_TRAP_TALKING,
    STATE_FOREST_PLAYING,
    STATE_FOREST_SCYLLA_BUTTON,
    STATE_FOREST_TALKING_AFTER_HURT,
    STATE_FOREST_TALKING_GAME_OVER,
    STATE_FOREST_WAIT_INTRO,
    STATE_FOREST_WIN_TALKING,
    STATE_FOREST_END
} ForestState;

typedef enum {
    OBS_NONE = 0,
    OBS_CATAPULT = 1,
    OBS_TRAP = 2,
    OBS_ROCK = 3,
    OBS_TREE = 4
} ObstacleType;

typedef struct {
    int score;
    int lives;
    double parallax_pos;
    ObstacleType obstacles[FOREST_MAX_TIME];
    int sacks[FOREST_MAX_TIME];
    int leaves[FOREST_MAX_TIME];
    bool arrow_up_focus;
    bool arrow_down_focus;
    double hugo_jumping_time;
    double hugo_crawling_time;
    double last_time;
    int old_second;
    
    // Cave game state (shared context)
    int cave_selected_rope;
    int cave_win_type;
    int rolling_score;
} ForestContext;

#endif