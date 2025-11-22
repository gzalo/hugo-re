#ifndef STATE_H
#define STATE_H

typedef struct {
    bool audio_played[4];
    double audio_start_time[4];
    double start_time;
} StateMetadata;

double get_state_time(StateMetadata* metadata);
int get_frame_index(StateMetadata* metadata);
int get_frame_index_fast(StateMetadata* metadata);
double get_game_time();

bool one_shot(StateMetadata* metadata, double delta, int idx);
bool every(StateMetadata* metadata, double delta, int idx, double offset);
void reset_state(StateMetadata* metadata);

typedef struct {
    bool key_up;
    bool key_down;
    bool key_start;
    bool cave_rope_1_pressed;
    bool cave_rope_2_pressed;
    bool cave_rope_3_pressed;
} InputState;


#endif