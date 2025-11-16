#include "common.h"
#include "state.h"

StateInfo state_info;

double get_state_time() {
    return (SDL_GetTicks() / 1000.0) - state_info.state_start_time;
}

int get_frame_index() {
    return (int)(get_state_time() * 10);
}

int get_frame_index_fast() {
    return (int)(get_state_time() * 20);
}

double get_game_time() {
    return SDL_GetTicks() / 1000.0;
}