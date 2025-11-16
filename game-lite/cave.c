#include "common.h"
#include <stdbool.h>
#include "state.h"

// Cave game processing functions
GameState process_cave_waiting_before_talking(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played) {
        if (audio.cave_her_er_vi) Mix_PlayChannel(-1, audio.cave_her_er_vi, 0);
        if (audio.cave_stemning) Mix_PlayChannel(-1, audio.cave_stemning, 0);
        audio_played = true;
    }
    
    if (get_state_time() > 2.5) {
        audio_played = false;
        return STATE_CAVE_TALKING_BEFORE_CLIMB;
    }
    return STATE_NONE;
}

GameState process_cave_talking_before_climb(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played && audio.cave_trappe_snak) {
        Mix_PlayChannel(-1, audio.cave_trappe_snak, 0);
        audio_played = true;
    }
    
    // Hugo talks before climbing (~4 seconds based on sync timing)
    if (get_state_time() > 4.0) {
        audio_played = false;
        return STATE_CAVE_CLIMBING;
    }
    return STATE_NONE;
}

GameState process_cave_intro(InputState state) {
    if (get_state_time() > 2.5) {
        return STATE_CAVE_CLIMBING;
    }
    return STATE_NONE;
}

GameState process_cave_climbing(InputState state) {
    if (get_state_time() > 5.1) {
        return STATE_CAVE_WAITING_INPUT;
    }
    return STATE_NONE;
}

GameState process_cave_waiting_input(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played && audio.cave_afskylia_snak) {
        Mix_PlayChannel(-1, audio.cave_afskylia_snak, 0);
        audio_played = true;
    }
    
    /*if (event && event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_3) {
            if (audio.cave_tast_trykket) Mix_PlayChannel(-1, audio.cave_tast_trykket, 0);
            game_ctx.cave_selected_rope = 0;
            audio_played = false;
            return STATE_CAVE_GOING_ROPE;
            return true;
        } else if (event->key.keysym.sym == SDLK_6) {
            if (audio.cave_tast_trykket) Mix_PlayChannel(-1, audio.cave_tast_trykket, 0);
            game_ctx.cave_selected_rope = 1;
            audio_played = false;
            return STATE_CAVE_GOING_ROPE;
            return true;
        } else if (event->key.keysym.sym == SDLK_9) {
            if (audio.cave_tast_trykket) Mix_PlayChannel(-1, audio.cave_tast_trykket, 0);
            game_ctx.cave_selected_rope = 2;
            audio_played = false;
            return STATE_CAVE_GOING_ROPE;
            return true;
        }
    }*/
    return STATE_NONE;
}

GameState process_cave_going_rope(InputState state) {
    double durations[] = {3.2, 4.0, 4.9};
    double sound_times[] = {2.0, 3.0, 4.0};
    
    // Play hiv_i_reb sound at specific time
    static bool hiv_played = false;
    if (!hiv_played && get_state_time() > sound_times[game_ctx.cave_selected_rope] && audio.cave_hiv_i_reb) {
        Mix_PlayChannel(-1, audio.cave_hiv_i_reb, 0);
        hiv_played = true;
    }
    
    // Play footstep sounds every 0.4 seconds
    static double last_footstep1 = 0;
    static double last_footstep2 = 0.4;
    if (get_state_time() - last_footstep1 >= 0.8) {
        if (audio.cave_fodtrin1) Mix_PlayChannel(-1, audio.cave_fodtrin1, 0);
        last_footstep1 = get_state_time();
    }
    if (get_state_time() - last_footstep2 >= 0.8) {
        if (audio.cave_fodtrin2) Mix_PlayChannel(-1, audio.cave_fodtrin2, 0);
        last_footstep2 = get_state_time();
    }
    
    if (get_state_time() > durations[game_ctx.cave_selected_rope]) {
        hiv_played = false;
        last_footstep1 = 0;
        last_footstep2 = 0.4;
        
        // Calculate win or lose (25% lose, 75% win)
        int random_value = rand() % 4;
        
        if (random_value == 0) {
            // Lost
            return STATE_CAVE_LOST;
        } else {
            // Won - different multipliers (Scylla loses, Hugo wins)
            if (random_value == 1) {
                game_ctx.cave_win_type = 0; // Bird
                game_ctx.score *= 2;
            } else if (random_value == 2) {
                game_ctx.cave_win_type = 1; // Leaves
                game_ctx.score *= 3;
            } else {
                game_ctx.cave_win_type = 2; // Ropes
                game_ctx.score *= 4;
            }
            return STATE_CAVE_SCYLLA_LOST;
        }
    }
    return STATE_NONE;
}

GameState process_cave_lost(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played) {
        if (audio.cave_pre_puf) Mix_PlayChannel(-1, audio.cave_pre_puf, 0);
        audio_played = true;
    }
    
    // Play puf sound at 1 second (adjusted based on typical timing)
    static bool puf_played = false;
    if (!puf_played && get_state_time() > 2.0 && audio.cave_puf) {
        Mix_PlayChannel(-1, audio.cave_puf, 0);
        if (audio.cave_hugo_skyd_ud) Mix_PlayChannel(-1, audio.cave_hugo_skyd_ud, 0);
        puf_played = true;
    }
    
    // Hugo animation based on selected rope
    double durations[] = {4.5, 4.5, 4.5};
    if (get_state_time() > durations[game_ctx.cave_selected_rope]) {
        audio_played = false;
        puf_played = false;
        return STATE_CAVE_LOST_SPRING;
    }
    return STATE_NONE;
}

GameState process_cave_lost_spring(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played && audio.cave_fjeder) {
        Mix_PlayChannel(-1, audio.cave_fjeder, 0);
        audio_played = true;
    }
    
    if (get_state_time() > 3.9) {
        audio_played = false;
        return STATE_END;
    }
    return STATE_NONE;
}

GameState process_cave_scylla_lost(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played) {
        if (game_ctx.cave_win_type == 0 && audio.cave_fugle_skrig) {
            // Bird sound
            Mix_PlayChannel(-1, audio.cave_fugle_skrig, 0);
        } else if (game_ctx.cave_win_type == 1 && audio.cave_skrig) {
            // Leaves sound
            Mix_PlayChannel(-1, audio.cave_skrig, 0);
        } else if (game_ctx.cave_win_type == 2) {
            // Ropes sounds
            if (audio.cave_pre_puf) Mix_PlayChannel(-1, audio.cave_pre_puf, 0);
        }
        if (audio.cave_afskylia_skyd_ud) Mix_PlayChannel(-1, audio.cave_afskylia_skyd_ud, 0);
        audio_played = true;
    }
    
    // Play puf for ropes at 1 second
    static bool puf_played = false;
    if (!puf_played && game_ctx.cave_win_type == 2 && get_state_time() > 2.0 && audio.cave_puf) {
        Mix_PlayChannel(-1, audio.cave_puf, 0);
        puf_played = true;
    }
    
    // Scylla loses animation (Hugo wins)
    double durations[] = {6.3, 5.6, 4.3}; // bird, leaves, ropes
    
    if (get_state_time() > durations[game_ctx.cave_win_type]) {
        audio_played = false;
        puf_played = false;
        if (game_ctx.cave_win_type == 2) {
            // Only ropes path goes through spring
            return STATE_CAVE_SCYLLA_SPRING;
        } else {
            return STATE_CAVE_FAMILY_CAGE_OPENS;
        }
    }
    return STATE_NONE;
}

GameState process_cave_scylla_spring(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played) {
        if (audio.cave_fjeder) Mix_PlayChannel(-1, audio.cave_fjeder, 0);
        if (audio.cave_afskylia_skyd_ud) Mix_PlayChannel(-1, audio.cave_afskylia_skyd_ud, 0);
        audio_played = true;
    }
    
    // Scylla spring animation
    if (get_state_time() > 3.5) {
        audio_played = false;
        return STATE_CAVE_FAMILY_CAGE_OPENS;
    }
    return STATE_NONE;
}

GameState process_cave_family_cage_opens(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played) {
        if (audio.cave_hiv_i_reb) Mix_PlayChannel(-1, audio.cave_hiv_i_reb, 0);
        audio_played = true;
    }
    
    // Play hugoline_tak at 0.5 seconds
    static bool hugoline_played = false;
    if (!hugoline_played && get_state_time() > 1.0 && audio.cave_hugoline_tak) {
        Mix_PlayChannel(-1, audio.cave_hugoline_tak, 0);
        hugoline_played = true;
    }
    
    // Family cage opens
    if (get_state_time() > 3.0) {
        audio_played = false;
        hugoline_played = false;
        return STATE_CAVE_FAMILY_HAPPY;
    }
    return STATE_NONE;
}

GameState process_cave_family_happy(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played && audio.cave_fanfare) {
        Mix_PlayChannel(-1, audio.cave_fanfare, 0);
        audio_played = true;
    }
    
    if (get_state_time() > 5.0) {
        audio_played = false;
        return STATE_END;
    }
    return STATE_NONE;
}