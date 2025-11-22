#include "common.h"
#include "state.h"
#include "render_sdl.h"

double get_game_time() {
    return get_time_ms() / 1000.0;
}

double get_state_time(StateMetadata* metadata) {
    return get_game_time() - metadata->start_time;
}

int get_frame_index(StateMetadata* metadata) {
    return (int)(get_state_time(metadata) * 10);
}

int get_frame_index_fast(StateMetadata* metadata) {
    return (int)(get_state_time(metadata) * 20);
}

void reset_state(StateMetadata* metadata){
    for(int i = 0; i < 4; i++){
        metadata->audio_played[i] = false;
        metadata->audio_start_time[i] = 0.0;
    }
    metadata->start_time = get_game_time();
}

bool one_shot(StateMetadata* metadata, double delta, int idx){
    double current_time = get_state_time(metadata);

    if(!metadata->audio_played[idx] && current_time > delta){
        metadata->audio_played[idx] = true;
        metadata->audio_start_time[idx] = delta;
        return true;
    }
    return false;
}

bool every(StateMetadata* metadata, double delta, int idx, double offset){
    double current_time = get_state_time(metadata);

    if(!metadata->audio_played[idx]){
        if(current_time > offset){
            metadata->audio_played[idx] = true;
            metadata->audio_start_time[idx] = current_time;
            return true;
        }
    } else if(current_time > metadata->audio_start_time[idx] + delta){
        metadata->audio_start_time[idx] = current_time;
        return true;
    }
    return false;
}
