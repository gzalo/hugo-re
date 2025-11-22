#ifndef CAVE_H
#define CAVE_H

#include "common.h"

GameState process_cave(InputState state);
void render_cave();
void on_enter_cave();
void set_cave_score(int score);

#endif
