#include "common.h"
#include "state.h"
#include "render_sdl.h"

double get_game_time() {
    return get_time_ms() / 1000.0;
}

double get_state_time(double state_start_time) {
    return get_game_time() - state_start_time;
}

int get_frame_index(double state_start_time) {
    return (int)(get_state_time(state_start_time) * 10);
}

int get_frame_index_fast(double state_start_time) {
    return (int)(get_state_time(state_start_time) * 20);
}

bool audio_played[4] = {false, false, false, false};
double audio_start_time[4] = {0.0, 0.0, 0.0, 0.0};

void reset_state_events(){
    for(int i = 0; i < 4; i++){
        audio_played[i] = false;
        audio_start_time[i] = 0.0;
    }
}

bool one_shot(double state_start_time, double delta, int idx){
    double current_time = get_state_time(state_start_time);

    if(!audio_played[idx] && current_time > delta){
        audio_played[idx] = true;
        audio_start_time[idx] = delta;
        return true;
    }
    return false;
}

bool every(double state_start_time, double delta, int idx, double offset){
    double current_time = get_state_time(state_start_time);

    if(!audio_played[idx]){
        if(current_time > offset){
            audio_played[idx] = true;
            audio_start_time[idx] = current_time;
            return true;
        }
    } else if(current_time > audio_start_time[idx] + delta){
        audio_start_time[idx] = current_time;
        return true;
    }
    return false;
}
