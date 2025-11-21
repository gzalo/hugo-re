#include "common.h"
#include "state.h"
#include "render_sdl.h"

StateInfo state_info;

double get_game_time() {
    return get_time_ms() / 1000.0;
}

double get_state_time() {
    return get_game_time() - state_info.state_start_time;
}

int get_frame_index() {
    return (int)(get_state_time() * 10);
}

int get_frame_index_fast() {
    return (int)(get_state_time() * 20);
}

bool audio_played[4] = {false, false, false, false};
double audio_start_time[4] = {0.0, 0.0, 0.0, 0.0};

void reset_state_events(){
    for(int i = 0; i < 4; i++){
        audio_played[i] = false;
        audio_start_time[i] = 0.0;
    }
}

bool one_shot(double delta, int idx){
    if(!audio_played[idx]){
        audio_played[idx] = true;
        audio_start_time[idx] = delta;
        return true;
    }
    return false;
}

bool every(double delta, int idx, double offset){
    double current_time = get_state_time();

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
