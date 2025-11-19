#include "common.h"
#include <stdbool.h>
#include "state.h"

// Cave game processing functions
GameState process_cave_waiting_before_talking(InputState state) { 
    if (get_state_time() > 2.5) {
        return STATE_CAVE_TALKING_BEFORE_CLIMB;
    }
    return STATE_NONE;
}

// Cave game rendering functions
void render_cave_waiting_before_talking() {
    render(textures.cave_talks.frames[12], 0, 0);
}

void on_enter_cave_talking_before_climb() {
    play(audio.cave_her_er_vi);
    play(audio.cave_stemning);
}

//--------------------------

GameState process_cave_talking_before_climb(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played && audio.cave_trappe_snak) {
        play(audio.cave_trappe_snak);
        audio_played = true;
    }
    
    // Hugo talks before climbing (~4 seconds based on sync timing)
    if (get_state_time() > 4.0) {
        audio_played = false;
        return STATE_CAVE_CLIMBING;
    }
    return STATE_NONE;
}


void render_cave_talking_before_climb() {
    Texture* texture = animation_get_sync_frame(textures.cave_talks, get_frame_index());
    render(texture,0,0);
}

//--------------------------

void on_enter_cave_climbing(){
    
}
void on_enter_cave_family_cage_opens(){
    
}
void on_enter_cave_family_happy(){
    
}
void on_enter_cave_going_rope(){
    
}
void on_enter_cave_lost(){
    
}
void on_enter_cave_lost_spring(){
    
}
void on_enter_cave_scylla_lost(){
    
}
void on_enter_cave_scylla_spring(){
    
}
void on_enter_cave_waiting_before_talking(){
    
}
void on_enter_cave_waiting_input(){
    
}

void render_cave_climbing() {
    
    if (textures.cave_climbs.frames && textures.cave_climbs.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_climbs.frame_count) frame = textures.cave_climbs.frame_count - 1;
        if (textures.cave_climbs.frames[frame]) {
            render(textures.cave_climbs.frames[frame],0,0);
        }
    }
}

void render_cave_waiting_input() {
    render(textures.cave_first_rope.frames[0],0,0);
}

void render_cave_going_rope() {
    
    Texture** rope_animation = NULL;
    int rope_count = 0;
    
    if (game_ctx.cave_selected_rope == 0) {
        rope_animation = textures.cave_first_rope.frames;
        rope_count = textures.cave_first_rope.frame_count;
    } else if (game_ctx.cave_selected_rope == 1) {
        rope_animation = textures.cave_second_rope.frames;
        rope_count = textures.cave_second_rope.frame_count;
    } else if (game_ctx.cave_selected_rope == 2) {
        rope_animation = textures.cave_third_rope.frames;
        rope_count = textures.cave_third_rope.frame_count;
    }
    
    if (rope_animation && rope_count > 0) {
        int frame = get_frame_index();
        if (frame >= rope_count) frame = rope_count - 1;
        if (rope_animation[frame]) {
            render(rope_animation[frame],0,0);
        }
    }
}

void render_cave_lost() {
    
    Texture** puff_animation = NULL;
    int puff_count = 0;
    
    if (game_ctx.cave_selected_rope == 0) {
        puff_animation = textures.cave_hugo_puff_first.frames;
        puff_count = textures.cave_hugo_puff_first.frame_count;
    } else if (game_ctx.cave_selected_rope == 1) {
        puff_animation = textures.cave_hugo_puff_second.frames;
        puff_count = textures.cave_hugo_puff_second.frame_count;
    } else if (game_ctx.cave_selected_rope == 2) {
        puff_animation = textures.cave_hugo_puff_third.frames;
        puff_count = textures.cave_hugo_puff_third.frame_count;
    }
    
    if (puff_animation && puff_count > 0) {
        int frame = get_frame_index();
        if (frame >= puff_count) frame = puff_count - 1;
        if (puff_animation[frame]) {
            render(puff_animation[frame],0,0);
        }
    }
}

void render_cave_lost_spring() {
    
    if (textures.cave_hugo_spring.frames && textures.cave_hugo_spring.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_hugo_spring.frame_count) frame = textures.cave_hugo_spring.frame_count - 1;
        if (textures.cave_hugo_spring.frames[frame]) {
            render(textures.cave_hugo_spring.frames[frame],0,0);
        }
    }
}

void render_cave_scylla_lost() {
    // Scylla loses (Hugo wins)
    
    Texture** scylla_animation = NULL;
    int scylla_count = 0;
    
    if (game_ctx.cave_win_type == 0) {
        scylla_animation = textures.cave_scylla_bird.frames;
        scylla_count = textures.cave_scylla_bird.frame_count;
    } else if (game_ctx.cave_win_type == 1) {
        scylla_animation = textures.cave_scylla_leaves.frames;
        scylla_count = textures.cave_scylla_leaves.frame_count;
    } else if (game_ctx.cave_win_type == 2) {
        scylla_animation = textures.cave_scylla_ropes.frames;
        scylla_count = textures.cave_scylla_ropes.frame_count;
    }
    
    if (scylla_animation && scylla_count > 0) {
        int frame = get_frame_index();
        if (frame >= scylla_count) frame = scylla_count - 1;
        if (scylla_animation[frame]) {
            render(scylla_animation[frame],0,0);
        }
    }
    
    // Render Hugo sprite on top
    if (textures.cave_hugo_sprite) {
        int hugo_positions[3][2] = {{25, 105}, {97, 100}, {172, 102}};
        render(textures.cave_hugo_sprite, hugo_positions[game_ctx.cave_selected_rope][0], hugo_positions[game_ctx.cave_selected_rope][1]);
    }
}

void render_cave_scylla_spring() {
    
    if (textures.cave_scylla_spring.frames && textures.cave_scylla_spring.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_scylla_spring.frame_count) frame = textures.cave_scylla_spring.frame_count - 1;
        if (textures.cave_scylla_spring.frames[frame]) {
            render(textures.cave_scylla_spring.frames[frame],0,0);
        }
    }
}

void render_cave_family_cage_opens() {
    
    if (textures.cave_family_cage.frames && textures.cave_family_cage.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_family_cage.frame_count) frame = textures.cave_family_cage.frame_count - 1;
        if (textures.cave_family_cage.frames[frame]) {
            render(textures.cave_family_cage.frames[frame],0,0);
        }
    }
}

void render_cave_family_happy() {
    
    if (textures.cave_happy.frames && textures.cave_happy.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_happy.frame_count) frame = textures.cave_happy.frame_count - 1;
        if (textures.cave_happy.frames[frame]) {
            render(textures.cave_happy.frames[frame],0,0);
        }
    }
}

// Legacy cave rendering functions (for compatibility)
void render_cave_win() {
    render_cave_scylla_lost();
}

void render_cave_win_spring() {
    render_cave_scylla_spring();
}

void render_cave_happy() {
    render_cave_family_happy();
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
        play(audio.cave_afskylia_snak);
        audio_played = true;
    }
    
    /*if (event && event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_3) {
            if (audio.cave_tast_trykket) play(audio.cave_tast_trykket);
            game_ctx.cave_selected_rope = 0;
            audio_played = false;
            return STATE_CAVE_GOING_ROPE;
            return true;
        } else if (event->key.keysym.sym == SDLK_6) {
            if (audio.cave_tast_trykket) play(audio.cave_tast_trykket);
            game_ctx.cave_selected_rope = 1;
            audio_played = false;
            return STATE_CAVE_GOING_ROPE;
            return true;
        } else if (event->key.keysym.sym == SDLK_9) {
            if (audio.cave_tast_trykket) play(audio.cave_tast_trykket);
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
        play(audio.cave_hiv_i_reb);
        hiv_played = true;
    }
    
    // Play footstep sounds every 0.4 seconds
    static double last_footstep1 = 0;
    static double last_footstep2 = 0.4;
    if (get_state_time() - last_footstep1 >= 0.8) {
        if (audio.cave_fodtrin1) play(audio.cave_fodtrin1);
        last_footstep1 = get_state_time();
    }
    if (get_state_time() - last_footstep2 >= 0.8) {
        if (audio.cave_fodtrin2) play(audio.cave_fodtrin2);
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
        if (audio.cave_pre_puf) play(audio.cave_pre_puf);
        audio_played = true;
    }
    
    // Play puf sound at 1 second (adjusted based on typical timing)
    static bool puf_played = false;
    if (!puf_played && get_state_time() > 2.0 && audio.cave_puf) {
        play(audio.cave_puf);
        if (audio.cave_hugo_skyd_ud) play(audio.cave_hugo_skyd_ud);
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
        play(audio.cave_fjeder);
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
            play(audio.cave_fugle_skrig);
        } else if (game_ctx.cave_win_type == 1 && audio.cave_skrig) {
            // Leaves sound
            play(audio.cave_skrig);
        } else if (game_ctx.cave_win_type == 2) {
            // Ropes sounds
            if (audio.cave_pre_puf) play(audio.cave_pre_puf);
        }
        if (audio.cave_afskylia_skyd_ud) play(audio.cave_afskylia_skyd_ud);
        audio_played = true;
    }
    
    // Play puf for ropes at 1 second
    static bool puf_played = false;
    if (!puf_played && game_ctx.cave_win_type == 2 && get_state_time() > 2.0 && audio.cave_puf) {
        play(audio.cave_puf);
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
        if (audio.cave_fjeder) play(audio.cave_fjeder);
        if (audio.cave_afskylia_skyd_ud) play(audio.cave_afskylia_skyd_ud);
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
        if (audio.cave_hiv_i_reb) play(audio.cave_hiv_i_reb);
        audio_played = true;
    }
    
    // Play hugoline_tak at 0.5 seconds
    static bool hugoline_played = false;
    if (!hugoline_played && get_state_time() > 1.0 && audio.cave_hugoline_tak) {
        play(audio.cave_hugoline_tak);
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
        play(audio.cave_fanfare);
        audio_played = true;
    }
    
    if (get_state_time() > 5.0) {
        audio_played = false;
        return STATE_END;
    }
    return STATE_NONE;
}