#ifndef STATE_H
#define STATE_H

double get_state_time();
int get_frame_index();
int get_frame_index_fast();
double get_game_time();
bool one_shot(double delta, int idx);
bool every(double delta, int idx, double offset);
void reset_state_events();

typedef struct {
    bool key_up;
    bool key_down;
    bool key_start;
    bool cave_rope_1_pressed;
    bool cave_rope_2_pressed;
    bool cave_rope_3_pressed;
} InputState;

#endif