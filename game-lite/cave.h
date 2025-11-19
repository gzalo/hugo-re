#ifndef CAVE_H
#define CAVE_H

#include "common.h"

GameState process_cave_climbing(InputState state);
GameState process_cave_family_cage_opens(InputState state);
GameState process_cave_family_happy(InputState state);
GameState process_cave_going_rope(InputState state);
GameState process_cave_lost(InputState state);
GameState process_cave_lost_spring(InputState state);
GameState process_cave_scylla_lost(InputState state);
GameState process_cave_scylla_spring(InputState state);
GameState process_cave_talking_before_climb(InputState state);
GameState process_cave_waiting_before_talking(InputState state);
GameState process_cave_waiting_input(InputState state);

void render_cave_climbing();
void render_cave_family_cage_opens();
void render_cave_family_happy();
void render_cave_going_rope();
void render_cave_lost();
void render_cave_lost_spring();
void render_cave_scylla_lost();
void render_cave_scylla_spring();
void render_cave_talking_before_climb();
void render_cave_waiting_before_talking();
void render_cave_waiting_input();

void on_enter_cave_climbing();
void on_enter_cave_family_cage_opens();
void on_enter_cave_family_happy();
void on_enter_cave_going_rope();
void on_enter_cave_lost();
void on_enter_cave_lost_spring();
void on_enter_cave_scylla_lost();
void on_enter_cave_scylla_spring();
void on_enter_cave_talking_before_climb();
void on_enter_cave_waiting_before_talking();
void on_enter_cave_waiting_input();

#endif
