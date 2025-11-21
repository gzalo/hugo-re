#ifndef FOREST_H
#define FOREST_H

void init_game_context();

GameState process_forest_branch_animation(InputState state);
GameState process_forest_branch_talking(InputState state);
GameState process_forest_flying_falling(InputState state);
GameState process_forest_flying_falling_hang_animation(InputState state);
GameState process_forest_flying_falling_hang_talking(InputState state);
GameState process_forest_flying_start(InputState state);
GameState process_forest_flying_talking(InputState state);
GameState process_forest_rock_animation(InputState state);
GameState process_forest_rock_hit_animation(InputState state);
GameState process_forest_rock_talking(InputState state);
GameState process_forest_trap_animation(InputState state);
GameState process_forest_trap_talking(InputState state);
GameState process_forest_playing(InputState state);
GameState process_forest_scylla_button(InputState state);
GameState process_forest_talking_after_hurt(InputState state);
GameState process_forest_talking_game_over(InputState state);
GameState process_forest_wait_intro(InputState state);
GameState process_forest_win_talking(InputState state);

void render_forest_branch_animation();
void render_forest_branch_talking();
void render_forest_flying_falling();
void render_forest_flying_falling_hang_animation();
void render_forest_flying_falling_hang_talking();
void render_forest_flying_start();
void render_forest_flying_talking();
void render_forest_rock_animation();
void render_forest_rock_hit_animation();
void render_forest_rock_talking();
void render_forest_trap_animation();
void render_forest_trap_talking();
void render_forest_playing();
void render_forest_scylla_button();
void render_forest_talking_after_hurt();
void render_forest_talking_game_over();
void render_forest_wait_intro();
void render_forest_win_talking();

void on_enter_forest_playing();

#endif