#include "common.h"
#include <stdbool.h>
#include "state.h"

void render_cave_scoreboard(void) {
    if (game_ctx.rolling_score < game_ctx.score) {
        int diff = game_ctx.score - game_ctx.rolling_score;
        int step = diff > 10 ? 10 : diff;
        game_ctx.rolling_score += step;
    }

    int value = game_ctx.rolling_score;
    int thousands = value / 1000;
    int hundreds = (value % 1000) / 100;
    int tens = (value % 100) / 10;
    int ones = value % 10;

}

GameState process_cave_waiting_before_talking(InputState state) { 
    if (get_state_time() > 2.5) {
        return STATE_CAVE_TALKING_BEFORE_CLIMB;
    }
    return STATE_NONE;
}

void render_cave_waiting_before_talking() {
    render(textures.cave_talks.frames[12], 0, 0);
    render_cave_scoreboard();
}

void on_enter_cave_waiting_before_talking(){
    play(audio.cave_her_er_vi);
    game_ctx.rolling_score = game_ctx.score;
}

//--------------------------

GameState process_cave_talking_before_climb(InputState state) {
    // Play audio once at 4.0 seconds (not at start)
    if (one_shot(4.0, 0)) {
        play(audio.cave_trappe_grin);
    }
    
    if (get_frame_index() >= textures.cave_talks.sync_count) {
        return STATE_CAVE_CLIMBING;
    }
    return STATE_NONE;
}

void render_cave_talking_before_climb() {
    Texture* texture = animation_get_sync_frame(textures.cave_talks, get_frame_index());
    render(texture,0,0);
    render_cave_scoreboard();
}

void on_enter_cave_talking_before_climb() {
    play(audio.cave_trappe_snak);
}

//--------------------------

GameState process_cave_climbing(InputState state) {
    if (get_frame_index() >= textures.cave_climbs.frame_count) {
        return STATE_CAVE_WAITING_INPUT;
    }

    if(one_shot(1.0, 0)){
        play(audio.cave_nu_kommer_jeg);
    }
    if(one_shot(2.0, 1)){
        play(audio.cave_pre_fanfare);
    }

    return STATE_NONE;
}

void render_cave_climbing() {
    Texture* texture = animation_get_frame(textures.cave_climbs, get_frame_index());
    render(texture,0,0);
    render_cave_scoreboard();
}

void on_enter_cave_climbing(){
}

//--------------------------

GameState process_cave_waiting_input(InputState state) {
    if(one_shot(0.5, 0)){
        play(audio.cave_afskylia_snak);
    }

    if(state.cave_rope_1_pressed) {
        game_ctx.cave_selected_rope = 0;
        return STATE_CAVE_GOING_ROPE;
    } else if(state.cave_rope_2_pressed) {
        game_ctx.cave_selected_rope = 1;
        return STATE_CAVE_GOING_ROPE;
    } else if(state.cave_rope_3_pressed) {
        game_ctx.cave_selected_rope = 2;
        return STATE_CAVE_GOING_ROPE;
    }
    return STATE_NONE;
}

void render_cave_waiting_input() {
    render(textures.cave_first_rope.frames[0],0,0);
    render_cave_scoreboard();
}

void on_enter_cave_waiting_input(){
    play(audio.cave_tast_trykket);
}

//--------------------------

Animation get_rope_animation(){
    if (game_ctx.cave_selected_rope == 0) {
        return textures.cave_first_rope;
    } else if (game_ctx.cave_selected_rope == 1) {
        return textures.cave_second_rope;
    } else if (game_ctx.cave_selected_rope == 2) {
        return textures.cave_third_rope;
    }
    return (Animation){0};
}

GameState process_cave_going_rope(InputState state) {
    // Play hiv_i_reb sound at specific time based on selected rope
    double sound_times[] = {2.0, 3.0, 4.0};
    if (one_shot(sound_times[game_ctx.cave_selected_rope], 0)) {
        play(audio.cave_hiv_i_reb);
    }
    
    // Footstep sounds alternating every 0.4 seconds
    if (every(0.4, 0, 0.0)) {
        play(audio.cave_fodtrin1);
    }
    if (every(0.4, 1, 0.4)) {
        play(audio.cave_fodtrin2);
    }
    
    if (get_frame_index() >= get_rope_animation().frame_count) {
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

void render_cave_going_rope() {   
    Texture *frame = animation_get_frame(get_rope_animation(), get_frame_index());
    render(frame, 0, 0);
    render_cave_scoreboard();
}

void on_enter_cave_going_rope(){
}

//--------------------------

void on_enter_cave_family_cage_opens(){
}

void on_enter_cave_family_happy(){
    play(audio.cave_fanfare);
}

void on_enter_cave_lost(){
}

void on_enter_cave_lost_spring(){
    play(audio.cave_hugo_katapult);
}

void on_enter_cave_scylla_lost(){
}

void on_enter_cave_scylla_spring(){
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
    render_cave_scoreboard();
}

void render_cave_lost_spring() {
    if (textures.cave_hugo_spring.frames && textures.cave_hugo_spring.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_hugo_spring.frame_count) frame = textures.cave_hugo_spring.frame_count - 1;
        if (textures.cave_hugo_spring.frames[frame]) {
            render(textures.cave_hugo_spring.frames[frame],0,0);
        }
    }
    render_cave_scoreboard();
}

void render_cave_scylla_lost() {
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
    render_cave_scoreboard();
}

void render_cave_scylla_spring() {
    if (textures.cave_scylla_spring.frames && textures.cave_scylla_spring.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_scylla_spring.frame_count) frame = textures.cave_scylla_spring.frame_count - 1;
        if (textures.cave_scylla_spring.frames[frame]) {
            render(textures.cave_scylla_spring.frames[frame],0,0);
        }
    }
    render_cave_scoreboard();
}

void render_cave_family_cage_opens() {
    if (textures.cave_family_cage.frames && textures.cave_family_cage.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_family_cage.frame_count) frame = textures.cave_family_cage.frame_count - 1;
        if (textures.cave_family_cage.frames[frame]) {
            render(textures.cave_family_cage.frames[frame],0,0);
        }
    }
    render_cave_scoreboard();
}

void render_cave_family_happy() {
    if (textures.cave_happy.frames && textures.cave_happy.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_happy.frame_count) frame = textures.cave_happy.frame_count - 1;
        if (textures.cave_happy.frames[frame]) {
            render(textures.cave_happy.frames[frame],0,0);
        }
    }
    render_cave_scoreboard();
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

GameState process_cave_lost(InputState state) {
    if (one_shot(1.0, 0)) {
        play(audio.cave_pre_puf);
    }
    
    if (one_shot(2.0, 1)) {
        play(audio.cave_puf);
    }
    
    // Use the correct animation length for the lose state
    Animation puff_animation = (Animation){0};
    if (game_ctx.cave_selected_rope == 0) {
        puff_animation = textures.cave_hugo_puff_first;
    } else if (game_ctx.cave_selected_rope == 1) {
        puff_animation = textures.cave_hugo_puff_second;
    } else if (game_ctx.cave_selected_rope == 2) {
        puff_animation = textures.cave_hugo_puff_third;
    }
    
    if (get_frame_index() >= puff_animation.frame_count) {
        return STATE_CAVE_LOST_SPRING;
    }
    return STATE_NONE;
}

GameState process_cave_lost_spring(InputState state) {
    if (one_shot(2.5, 0)) {
        play(audio.cave_hugo_skyd_ud);
    }
    
    if (get_frame_index() >= textures.cave_hugo_spring.frame_count) {
        return STATE_END;
    }
    return STATE_NONE;
}

GameState process_cave_scylla_lost(InputState state) {
    if (game_ctx.cave_win_type == 0) {
        // Bird sound
        if (one_shot(0.5, 0)) {
            play(audio.cave_fugle_skrig);
        }
    } else if (game_ctx.cave_win_type == 1) {
        // Leaves sound
        if (one_shot(0.5, 0)) {
            play(audio.cave_skrig);
        }
    } else if (game_ctx.cave_win_type == 2) {
        // Ropes sounds
        if (one_shot(1.0, 0)) {
            play(audio.cave_pre_puf);
        }
        if (one_shot(2.0, 1)) {
            play(audio.cave_puf);
        }
    }
    
    // Use the correct animation length based on win type
    Animation scylla_animation = (Animation){0};
    if (game_ctx.cave_win_type == 0) {
        scylla_animation = textures.cave_scylla_bird;
    } else if (game_ctx.cave_win_type == 1) {
        scylla_animation = textures.cave_scylla_leaves;
    } else if (game_ctx.cave_win_type == 2) {
        scylla_animation = textures.cave_scylla_ropes;
    }
    
    if (get_frame_index() >= scylla_animation.frame_count) {
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
    if (one_shot(1.5, 0)) {
        play(audio.cave_fjeder);
    }
    
    if (one_shot(2.0, 1)) {
        play(audio.cave_afskylia_skyd_ud);
    }
    
    if (get_frame_index() >= textures.cave_scylla_spring.frame_count) {
        return STATE_CAVE_FAMILY_CAGE_OPENS;
    }
    return STATE_NONE;
}

GameState process_cave_family_cage_opens(InputState state) {
    if (one_shot(0.5, 0)) {
        play(audio.cave_hiv_i_reb);
    }
    
    if (one_shot(1.0, 1)) {
        play(audio.cave_hugoline_tak);
    }
    
    if (get_frame_index() >= textures.cave_family_cage.frame_count) {
        return STATE_CAVE_FAMILY_HAPPY;
    }
    return STATE_NONE;
}

GameState process_cave_family_happy(InputState state) {
    if (get_frame_index() >= textures.cave_happy.frame_count) {
        return STATE_END;
    }
    return STATE_NONE;
}