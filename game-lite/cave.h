#ifndef CAVE_H
#define CAVE_H

#include "common.h"

GameState process_cave(InputState state);
void render_cave();
void on_enter_cave();
void set_cave_score(int score);

typedef enum {
    STATE_CAVE_NONE,
    STATE_CAVE_CLIMBING,
    STATE_CAVE_FAMILY_CAGE_OPENS,
    STATE_CAVE_FAMILY_HAPPY,
    STATE_CAVE_GOING_ROPE,
    STATE_CAVE_LOST,
    STATE_CAVE_LOST_SPRING,
    STATE_CAVE_SCYLLA_LOST,
    STATE_CAVE_SCYLLA_SPRING,
    STATE_CAVE_TALKING_BEFORE_CLIMB,
    STATE_CAVE_WAITING_BEFORE_TALKING,
    STATE_CAVE_WAITING_INPUT,
    STATE_CAVE_END
} CaveState;

typedef struct {
    // Cave game data
    int cave_selected_rope;  // 0, 1, or 2 for ropes 3, 6, 9
    int cave_win_type;       // 0=bird, 1=leaves, 2=ropes
    int score;               // Current score
    int rolling_score;       // Score for display animation
} CaveContext;

// Internal state machine functions
void on_enter_cave_state(CaveState state);
void on_exit_cave_state(CaveState state);

// State-specific process functions
CaveState process_cave_waiting_before_talking(InputState state);
CaveState process_cave_talking_before_climb(InputState state);
CaveState process_cave_climbing(InputState state);
CaveState process_cave_waiting_input(InputState state);
CaveState process_cave_going_rope(InputState state);
CaveState process_cave_lost(InputState state);
CaveState process_cave_lost_spring(InputState state);
CaveState process_cave_scylla_lost(InputState state);
CaveState process_cave_scylla_spring(InputState state);
CaveState process_cave_family_cage_opens(InputState state);
CaveState process_cave_family_happy(InputState state);

// State-specific render functions
void render_cave_waiting_before_talking();
void render_cave_talking_before_climb();
void render_cave_climbing();
void render_cave_waiting_input();
void render_cave_going_rope();
void render_cave_lost();
void render_cave_lost_spring();
void render_cave_scylla_lost();
void render_cave_scylla_spring();
void render_cave_family_cage_opens();
void render_cave_family_happy();
void render_cave_scoreboard();

// State-specific on_enter functions
void on_enter_cave_waiting_before_talking();
void on_enter_cave_talking_before_climb();
void on_enter_cave_climbing();
void on_enter_cave_waiting_input();
void on_enter_cave_going_rope();
void on_enter_cave_lost();
void on_enter_cave_lost_spring();
void on_enter_cave_scylla_lost();
void on_enter_cave_scylla_spring();
void on_enter_cave_family_happy();

// Helper functions
Animation get_rope_animation();

#endif
