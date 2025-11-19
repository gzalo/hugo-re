#include "common.h"
#include "stdbool.h"
#include <stdio.h>
#include "state.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void generate_obstacles() {
    // Empty 65%, others 8.75% each
    for (int i = 0; i < FOREST_MAX_TIME; i++) {
        int r = rand() % 100;
        if (r < 65) {
            game_ctx.obstacles[i] = OBS_NONE;
        } else if (r < 74) {
            game_ctx.obstacles[i] = OBS_CATAPULT;
        } else if (r < 83) {
            game_ctx.obstacles[i] = OBS_TRAP;
        } else if (r < 91) {
            game_ctx.obstacles[i] = OBS_ROCK;
        } else {
            game_ctx.obstacles[i] = OBS_TREE;
        }
    }
    
    // Clear first few positions
    game_ctx.obstacles[0] = OBS_NONE;
    game_ctx.obstacles[1] = OBS_NONE;
    game_ctx.obstacles[2] = OBS_NONE;
    game_ctx.obstacles[3] = OBS_NONE;
    
    // Ensure no consecutive obstacles
    for (int i = 0; i < FOREST_MAX_TIME - 1; i++) {
        if (game_ctx.obstacles[i] != OBS_NONE) {
            game_ctx.obstacles[i + 1] = OBS_NONE;
        }
    }
}

void generate_sacks() {
    for (int i = 0; i < FOREST_MAX_TIME; i++) {
        int r = rand() % 100;
        if (r < 70) {
            game_ctx.sacks[i] = 0; // No sack
        } else if (r < 91) {
            game_ctx.sacks[i] = 1; // Normal sack
        } else {
            game_ctx.sacks[i] = 2; // Gold sack
        }
    }
}


void init_game_context() {
    game_ctx.score = 123;
    game_ctx.lives = START_LIVES;
    game_ctx.parallax_pos = 0;
    game_ctx.arrow_up_focus = false;
    game_ctx.arrow_down_focus = false;
    game_ctx.hugo_jumping_time = -1;
    game_ctx.hugo_crawling_time = -1;
    game_ctx.last_time = get_game_time();
    game_ctx.old_second = -1;
    
    // Cave game initialization
    game_ctx.cave_selected_rope = -1;
    game_ctx.cave_win_type = -1;
    game_ctx.rolling_score = 0;
    
    generate_obstacles();
    generate_sacks();
}


// Render HUD
void render_hud() {
    // Scoreboard background
    render(textures.scoreboard, 0, 0);
    
    // Lives indicators
    for (int i = 0; i < game_ctx.lives; i++) {
        if (textures.hugo_lives) {
            render(textures.hugo_lives, 32 + i * 40, 188);
        }
    }
    
    // Score digits
    int x_score = 200;
    int y_score = 194;
    int x_space = 24;
    int thousands = game_ctx.score / 1000;
    int hundreds = (game_ctx.score % 1000) / 100;
    int tens = (game_ctx.score % 100) / 10;
    int ones = game_ctx.score % 10;
    int digits[4] = {thousands, hundreds, tens, ones};
    
    // Score numbers are in a spritesheet (5 columns, 2 rows for 0-9)
    for (int i = 0; i < 4; i++) {
        int digit = digits[i];
        int xpos = digit % 5;
        int ypos = digit / 5;
        render(textures.score_numbers, 0, 10);
    }
}

// Render obstacles
void render_obstacles() {
    int frame = get_frame_index();
    
    for (int i = 0; i < FOREST_MAX_TIME; i++) {
        double obstacle_pos = (i - game_ctx.parallax_pos) * FOREST_GROUND_SPEED;
        
        if (obstacle_pos < -50 || obstacle_pos > SCREEN_WIDTH + 50) {
            continue;
        }
        
        switch (game_ctx.obstacles[i]) {
            case OBS_CATAPULT:
                {
                    if (textures.catapult[0]) {
                        int cat_frame = frame % 8;
                        render(textures.catapult[cat_frame], 0, 0);
                    } else {
                    }
                }
                break;
            case OBS_TRAP:
                {
                    if (textures.trap[0]) {
                        int trap_frame = frame % 6;
                        render(textures.trap[trap_frame], 0, 0);
                    } else {
                    }
                }
                break;
            case OBS_ROCK:
                {
                    double fract = game_ctx.parallax_pos - floor(game_ctx.parallax_pos);
                    int bounce = (int)(sin(fract * 2 * M_PI) * 15);
                    if (textures.rock[0]) {
                        int rock_frame = frame % 8;
                        render(textures.rock[rock_frame], 0, 0);
                    } else {
                    }
                }
                break;
            case OBS_TREE:
                {
                    // Render tree trunk if available
                    if (textures.lone_tree) {
                        render(textures.lone_tree, 0, 0);
                    }
                    // Render swinging branch
                    if (textures.tree[0]) {
                        int tree_frame = frame % 7;
                        render(textures.tree[tree_frame], 0, 0);
                    } else {
                    }
                }
                break;
            default:
                break;
        }
    }
}

// Render sacks
void render_sacks() {
    int frame = get_frame_index();
    
    for (int i = 0; i < FOREST_MAX_TIME; i++) {
        if (game_ctx.sacks[i] == 0) continue;
        
        double sack_pos = (i - game_ctx.parallax_pos) * FOREST_GROUND_SPEED;
        
        if (sack_pos < -50 || sack_pos > SCREEN_WIDTH + 50) {
            continue;
        }
        
        int sack_frame = frame % 4;
        render(textures.sack[sack_frame], 0, 0);
    }
}

// Render Hugo
void render_hugo() {
    int hugo_y = 90;
    int frame = get_frame_index();
    
    if (game_ctx.arrow_up_focus && game_ctx.hugo_jumping_time >= 0) {
        // Jumping animation
        double dt = (get_game_time() - game_ctx.hugo_jumping_time) / 0.75;
        double dy = -250 * dt * dt + 250 * dt - 22.5;
        hugo_y = (int)(40 - dy);
        
        if (textures.hugo_jump[0]) {
            int jump_frame = frame % 3;
            render(textures.hugo_jump[jump_frame], 0, 0);
        } else {
        }
    } else if (game_ctx.arrow_down_focus && game_ctx.hugo_crawling_time >= 0) {
        // Crawling animation
        hugo_y = 105;
        if (textures.hugo_crawl[0]) {
            int crawl_frame = frame % 8;
            render(textures.hugo_crawl[crawl_frame], 0, 0);
        } else {
        }
    } else {
        // Normal running animation
        if (textures.hugo_side[0]) {
            int side_frame = frame % 8;
            render(textures.hugo_side[side_frame], 0, 0);
        } else {
        }
    }
}

// Render control indicators
void render_controls() {
    // Use actual arrow textures if available
    // arrows[0] = up normal, arrows[1] = up pressed
    // arrows[2] = down normal, arrows[3] = down pressed
    if (game_ctx.arrow_up_focus) {
        render(textures.arrows[1], 0, 0);
    } else {
        render(textures.arrows[0], 0, 0);
    }
        
    if (game_ctx.arrow_down_focus) {
        render(textures.arrows[3], 0, 0);
    } else {
        render(textures.arrows[2], 0, 0);
    }
}


// Render background (simplified)
void render_background() {
    // Render gradient or solid sky
    if (textures.bg_gradient) {
        render(textures.bg_gradient, 0, 0);
    } else {
    }
    
    // Render parallax background layers (hills, trees) if available
    if (textures.bg_hillsday) {
        // Simple parallax - move slower than foreground
        int hills_offset = (int)(game_ctx.parallax_pos * 6) % 320;
        render(textures.bg_hillsday, 0, 0);
        // Render second copy for seamless scroll
        render(textures.bg_hillsday, 0, 0);
    }
    
    if (textures.bg_trees) {
        int trees_offset = (int)(game_ctx.parallax_pos * 12) % 320;
        render(textures.bg_trees, 0, 0);
        render(textures.bg_trees, 0, 0);
    }
    
    // Render ground layer
    int ground_offset = (int)(game_ctx.parallax_pos * FOREST_GROUND_SPEED) % 320;
    for (int i = 0; i < 5; i++) {
        render(textures.bg_ground, 0, 0);
    }
        
    // Render grass layer
    int grass_offset = (int)(game_ctx.parallax_pos * 30) % 320;
    for (int i = 0; i < 12; i++) {
        render(textures.grass, 0, 0);
    }
}


// Render intro animation
void render_forest_wait_intro() {
    
    int frame = get_frame_index() % 16;
    render(textures.hugo_telllives[frame], 0, 0);
    
    // Lives indicator
    for (int i = 0; i < game_ctx.lives; i++) {
        if (textures.hugo_lives) {
            render(textures.hugo_lives, 0, 0);
        } else {
        }
    }
}

// Render playing state
void render_forest_playing() {
    render_background();
    render_obstacles();
    render_sacks();
    render_hugo();
    render_controls();
    render_hud();
}

// Forest hurt state rendering functions
void render_forest_branch_animation() {
    if (textures.hugohitlog.frames && textures.hugohitlog.frame_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.hugohitlog.frame_count) frame = textures.hugohitlog.frame_count - 1;
        if (textures.hugohitlog.frames[frame]) {
            render(textures.hugohitlog.frames[frame],0,0);
        }
    }
}

void render_forest_branch_talking() {
    
    if (textures.hugohitlog_talk.frames && textures.hugohitlog_talk.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.hugohitlog_talk.frame_count) frame = textures.hugohitlog_talk.frame_count - 1;
        if (textures.hugohitlog_talk.frames[frame]) {
            render(textures.hugohitlog_talk.frames[frame],0,0);
        }
    }
}

void render_forest_flying_start() {
    
    if (textures.catapult_fly.frames && textures.catapult_fly.frame_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.catapult_fly.frame_count) frame = textures.catapult_fly.frame_count - 1;
        if (textures.catapult_fly.frames[frame]) {
            render(textures.catapult_fly.frames[frame],0,0);
        }
    }
}

void render_forest_flying_talking() {
    
    if (textures.catapult_airtalk.frames && textures.catapult_airtalk.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.catapult_airtalk.frame_count) frame = textures.catapult_airtalk.frame_count - 1;
        if (textures.catapult_airtalk.frames[frame]) {
            render(textures.catapult_airtalk.frames[frame],0,0);
        }
    }
}

void render_forest_flying_falling() {
    
    if (textures.catapult_fall.frames && textures.catapult_fall.frame_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.catapult_fall.frame_count) frame = textures.catapult_fall.frame_count - 1;
        if (textures.catapult_fall.frames[frame]) {
            render(textures.catapult_fall.frames[frame],0,0);
        }
    }
}

void render_forest_flying_falling_hang_animation() {
    
    if (textures.catapult_hang.frames && textures.catapult_hang.frame_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.catapult_hang.frame_count) frame = textures.catapult_hang.frame_count - 1;
        if (textures.catapult_hang.frames[frame]) {
            render(textures.catapult_hang.frames[frame],0,0);
        }
    }
}

void render_forest_flying_falling_hang_talking() {
    
    // Render static hang frame as background
    if (textures.catapult_hang.frames && textures.catapult_hang.frame_count > 12) {
        if (textures.catapult_hang.frames[12]) {
            render(textures.catapult_hang.frames[12],0,0);
        }
    }
    
    // Render talking mouth animation on top
    if (textures.catapult_hangspeak.frames && textures.catapult_hangspeak.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.catapult_hangspeak.frame_count) frame = textures.catapult_hangspeak.frame_count - 1;
        if (textures.catapult_hangspeak.frames[frame]) {
            render(textures.catapult_hangspeak.frames[frame], 0, 0);
        }
    }
}

void render_forest_rock_animation() {
    
    if (textures.hugo_lookrock.frames && textures.hugo_lookrock.frame_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.hugo_lookrock.frame_count) frame = textures.hugo_lookrock.frame_count - 1;
        if (textures.hugo_lookrock.frames[frame]) {
            render(textures.hugo_lookrock.frames[frame],0,0);
        }
    }
}

void render_forest_rock_hit_animation() {
    
    if (textures.hit_rock.frames && textures.hit_rock.frame_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.hit_rock.frame_count) frame = textures.hit_rock.frame_count - 1;
        if (textures.hit_rock.frames[frame]) {
            render(textures.hit_rock.frames[frame],0,0);
        }
    }
}

void render_forest_rock_talking() {
    
    if (textures.hit_rock_sync.frames && textures.hit_rock_sync.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.hit_rock_sync.frame_count) frame = textures.hit_rock_sync.frame_count - 1;
        if (textures.hit_rock_sync.frames[frame]) {
            render(textures.hit_rock_sync.frames[frame],0,0);
        }
    }
}

void render_forest_trap_animation() {
    
    if (textures.hugo_traphurt.frames && textures.hugo_traphurt.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.hugo_traphurt.frame_count) frame = textures.hugo_traphurt.frame_count - 1;
        if (textures.hugo_traphurt.frames[frame]) {
            render(textures.hugo_traphurt.frames[frame],0,0);
        }
    }
}

void render_forest_trap_talking() {
    
    if (textures.hugo_traptalk.frames && textures.hugo_traptalk.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.hugo_traptalk.frame_count) frame = textures.hugo_traptalk.frame_count - 1;
        if (textures.hugo_traptalk.frames[frame]) {
            render(textures.hugo_traptalk.frames[frame],0,0);
        }
    }
}

void render_forest_scylla_button() {
    
    // Scylla button is shown in multiplayer mode - not typically used in single player
    // Just render a blank screen for now
}

void render_forest_talking_after_hurt() {
    
    // Render Hugo telllives animation
    if (textures.hugo_telllives[0]) {
        int frame = get_frame_index();
        if (frame >= 16) frame = 15;
        render(textures.hugo_telllives[frame], 0, 0);
    }
}

void render_forest_talking_game_over() {
    
    // Render Hugo telllives animation for game over
    if (textures.hugo_telllives[0]) {
        int frame = get_frame_index();
        if (frame >= 16) frame = 15;
        render(textures.hugo_telllives[frame], 0, 0);
    }
}

void render_forest_win_talking() {
    
    // Render Hugo telllives animation for win
    if (textures.hugo_telllives[0]) {
        int frame = get_frame_index();
        if (frame >= 16) frame = 15;
        render(textures.hugo_telllives[frame], 0, 0);
    }
}


// Process wait intro state
GameState process_forest_wait_intro(InputState state) {
    // Play intro speech once at start
    static bool intro_played = false;
    if (!intro_played && audio.speak_start) {
        play(audio.speak_start);
        intro_played = true;
    }
    
    if (get_state_time() > 2.0) {
        intro_played = false;  // Reset for next time
        return STATE_FOREST_PLAYING;
    }
    return STATE_NONE;
}

// Check collision with obstacle
bool check_collision(int obstacle_idx) {
    ObstacleType obs = game_ctx.obstacles[obstacle_idx];
    if (obs == OBS_NONE) return false;
    
    // Different obstacles require different actions
    if (obs == OBS_CATAPULT || obs == OBS_TRAP || obs == OBS_ROCK) {
        // Need to jump
        return !game_ctx.arrow_up_focus;
    } else if (obs == OBS_TREE) {
        // Need to duck
        return !game_ctx.arrow_down_focus;
    }
    
    return false;
}

// Process playing state
GameState process_forest_playing(InputState state) {
    // Handle input
    /*if (event && event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_2 || event->key.keysym.sym == SDLK_UP) {
            if (!game_ctx.arrow_up_focus && !game_ctx.arrow_down_focus) {
                game_ctx.arrow_up_focus = true;
                game_ctx.hugo_jumping_time = get_game_time();
            }
        } else if (event->key.keysym.sym == SDLK_8 || event->key.keysym.sym == SDLK_DOWN) {
            if (!game_ctx.arrow_down_focus && !game_ctx.arrow_up_focus) {
                game_ctx.arrow_down_focus = true;
                game_ctx.hugo_crawling_time = get_game_time();
            }
        }
    }*/
    
    // Check if reached end
    if (game_ctx.parallax_pos >= FOREST_MAX_TIME) {
        return STATE_FOREST_WIN_TALKING;
    }
    
    // Update position
    double current_time = get_game_time();
    game_ctx.parallax_pos += current_time - game_ctx.last_time;
    game_ctx.last_time = current_time;
    
    // Get current position
    int current_pos = (int)floor(game_ctx.parallax_pos) + 1;
    if (current_pos >= FOREST_MAX_TIME) {
        current_pos = FOREST_MAX_TIME - 1;
    }
    
    // Check for new obstacles
    if (game_ctx.old_second != (int)floor(game_ctx.parallax_pos)) {
        // Check collision with obstacles and transition to hurt states
        ObstacleType obs = game_ctx.obstacles[current_pos];
        if (obs != OBS_NONE) {
            if (obs == OBS_CATAPULT && !game_ctx.arrow_up_focus) {
                // Catapult hit - go flying
                if (audio.sfx_hugo_launch) play(audio.sfx_hugo_launch);
                if (audio.sfx_catapult_eject) play(audio.sfx_catapult_eject);
                game_ctx.obstacles[current_pos] = OBS_NONE;
                return STATE_FOREST_FLYING_START;
            } else if (obs == OBS_TRAP && !game_ctx.arrow_up_focus) {
                // Trap hit
                if (audio.sfx_hugo_hittrap) play(audio.sfx_hugo_hittrap);
                game_ctx.obstacles[current_pos] = OBS_NONE;
                return STATE_FOREST_TRAP_ANIMATION;
            } else if (obs == OBS_ROCK && !game_ctx.arrow_up_focus) {
                // Rock hit
                if (audio.sfx_hugo_hitlog) play(audio.sfx_hugo_hitlog);
                game_ctx.obstacles[current_pos] = OBS_NONE;
                return STATE_FOREST_ROCK_ANIMATION;
            } else if (obs == OBS_TREE && !game_ctx.arrow_down_focus) {
                // Branch hit (need to duck to avoid)
                if (audio.sfx_hugo_hitlog) play(audio.sfx_hugo_hitlog);
                game_ctx.obstacles[current_pos] = OBS_NONE;
                return STATE_FOREST_BRANCH_ANIMATION;
            } else if (obs == OBS_TREE && game_ctx.arrow_down_focus) {
                // Ducked under tree successfully
                if (audio.sfx_tree_swush) play(audio.sfx_tree_swush);
            }
        }
        
        // Check sack collection
        if (game_ctx.arrow_up_focus && game_ctx.sacks[current_pos] != 0) {
            if (game_ctx.sacks[current_pos] == 1) {
                game_ctx.score += 100;
                if (audio.sfx_sack_normal) play(audio.sfx_sack_normal);
            } else {
                game_ctx.score += 250;
                if (audio.sfx_sack_bonus) play(audio.sfx_sack_bonus);
            }
            game_ctx.sacks[current_pos] = 0;
            printf("Collected sack! Score: %d\n", game_ctx.score);
        }
        
        game_ctx.old_second = (int)floor(game_ctx.parallax_pos);
    }
    
    // Update jump/crawl timers
    if (game_ctx.arrow_up_focus && game_ctx.hugo_jumping_time >= 0) {
        if (current_time - game_ctx.hugo_jumping_time > 0.75) {
            game_ctx.hugo_jumping_time = -1;
            game_ctx.arrow_up_focus = false;
        }
    }
    
    if (game_ctx.arrow_down_focus && game_ctx.hugo_crawling_time >= 0) {
        if (current_time - game_ctx.hugo_crawling_time > 0.75) {
            game_ctx.hugo_crawling_time = -1;
            game_ctx.arrow_down_focus = false;
        }
    }
    return STATE_NONE;
}

// Helper function to reduce lives and transition appropriately
GameState reduce_lives_and_transition() {
    game_ctx.lives--;
    printf("Hit obstacle! Lives remaining: %d\n", game_ctx.lives);
    
    if (game_ctx.lives <= 0) {
        return STATE_FOREST_TALKING_GAME_OVER;
    } else {
        return STATE_FOREST_TALKING_AFTER_HURT;
    }
}

// Forest hurt state processing functions

GameState process_forest_branch_animation(InputState state) {
    // Play bird sounds once at start
    static bool audio_played = false;
    if (!audio_played && audio.sfx_birds) {
        play(audio.sfx_birds);
        audio_played = true;
    }
    
    // Branch hit animation - placeholder timing, ideally based on animation frames
    if (get_state_time() > 1.0) {
        audio_played = false;
        return STATE_FOREST_BRANCH_TALKING;
    }
    return STATE_NONE;
}

GameState process_forest_branch_talking(InputState state) {
    // Play speech once at start
    static bool audio_played = false;
    if (!audio_played && audio.speak_hitlog) {
        play(audio.speak_hitlog);
        audio_played = true;
    }
    
    // Branch talking - placeholder timing
    if (get_state_time() > 2.0) {
        audio_played = false;
        return reduce_lives_and_transition();
    }
    return STATE_NONE;
}

GameState process_forest_flying_start(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played) {
        if (audio.speak_catapult_up) play(audio.speak_catapult_up);
        audio_played = true;
    }
    
    // Play crash sound at 2.7 seconds
    static bool crash_played = false;
    if (!crash_played && get_state_time() > 2.7) {
        if (audio.sfx_hugo_screenklir) play(audio.sfx_hugo_screenklir);
        if (audio.speak_catapult_hit) play(audio.speak_catapult_hit);
        crash_played = true;
    }
    
    // Hugo flying up from catapult
    if (get_state_time() > 3.0) {
        audio_played = false;
        crash_played = false;
        return STATE_FOREST_FLYING_TALKING;
    }
    return STATE_NONE;
}

GameState process_forest_flying_talking(InputState state) {
    // Play speech once at start
    static bool audio_played = false;
    if (!audio_played && audio.speak_catapult_talktop) {
        play(audio.speak_catapult_talktop);
        audio_played = true;
    }
    
    // Hugo talking while at top of flight
    if (get_state_time() > 1.5) {
        audio_played = false;
        return STATE_FOREST_FLYING_FALLING;
    }
    return STATE_NONE;
}

GameState process_forest_flying_falling(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played) {
        if (audio.speak_catapult_down) play(audio.speak_catapult_down);
        if (audio.sfx_hugo_crash) play(audio.sfx_hugo_crash);
        audio_played = true;
    }
    
    // Hugo falling down
    if (get_state_time() > 1.0) {
        audio_played = false;
        return STATE_FOREST_FLYING_FALLING_HANG_ANIMATION;
    }
    return STATE_NONE;
}

GameState process_forest_flying_falling_hang_animation(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played && audio.sfx_hugo_hangstart) {
        play(audio.sfx_hugo_hangstart);
        audio_played = true;
    }
    
    // Hugo catching branch animation
    if (get_state_time() > 1.5) {
        audio_played = false;
        return STATE_FOREST_FLYING_FALLING_HANG_TALKING;
    }
    return STATE_NONE;
}

GameState process_forest_flying_falling_hang_talking(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played) {
        if (audio.speak_catapult_hang) play(audio.speak_catapult_hang);
        if (audio.sfx_hugo_hang) play(audio.sfx_hugo_hang);
        audio_played = true;
    }
    
    // Hugo talking while hanging
    if (get_state_time() > 2.0) {
        audio_played = false;
        return reduce_lives_and_transition();
    }
    return STATE_NONE;
}

GameState process_forest_rock_animation(InputState state) {
    // Hugo looking at rock
    if (get_state_time() > 0.5) {
        return STATE_FOREST_ROCK_HIT_ANIMATION;
    }
    return STATE_NONE;
}

GameState process_forest_rock_hit_animation(InputState state) {
    // Rock hits Hugo
    if (get_state_time() > 1.0) {
        return STATE_FOREST_ROCK_TALKING;
    }
    return STATE_NONE;
}

GameState process_forest_rock_talking(InputState state) {
    // Play speech once at start
    static bool audio_played = false;
    if (!audio_played && audio.speak_rock) {
        play(audio.speak_rock);
        audio_played = true;
    }
    
    // Hugo talks after rock hit
    if (get_state_time() > 2.0) {
        audio_played = false;
        return reduce_lives_and_transition();
    }
    return STATE_NONE;
}

GameState process_forest_trap_animation(InputState state) {
    // Hugo falling into trap
    if (get_state_time() > 1.5) {
        return STATE_FOREST_TRAP_TALKING;
    }
    return STATE_NONE;
}

GameState process_forest_trap_talking(InputState state) {
    // Play speech once at start
    static bool audio_played = false;
    if (!audio_played && audio.speak_trap) {
        play(audio.speak_trap);
        audio_played = true;
    }
    
    // Hugo talking after trap
    if (get_state_time() > 2.0) {
        audio_played = false;
        return reduce_lives_and_transition();
    }
    return STATE_NONE;
}

GameState process_forest_scylla_button(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played && audio.sfx_lightning_warning) {
        play(audio.sfx_lightning_warning);
        audio_played = true;
    }
    
    // Scylla button flashing (not normally triggered in single player)
    if (get_state_time() > 2.0) {
        audio_played = false;
        return STATE_FOREST_PLAYING;
    }
    return STATE_NONE;
}

GameState process_forest_talking_after_hurt(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played) {
        if (audio.sfx_hugo_knock) play(audio.sfx_hugo_knock);
        if (game_ctx.lives == 1 && audio.speak_lastlife) {
            play(audio.speak_lastlife);
        } else if (audio.speak_dieonce) {
            play(audio.speak_dieonce);
        }
        audio_played = true;
    }
    
    // Play knock again at 0.5 seconds
    static bool knock2_played = false;
    if (!knock2_played && get_state_time() > 0.5 && audio.sfx_hugo_knock) {
        play(audio.sfx_hugo_knock);
        knock2_played = true;
    }
    
    // Hugo talks after getting hurt (still has lives)
    double duration = (game_ctx.lives == 1) ? 3.0 : 2.5;
    if (get_state_time() > duration) {
        audio_played = false;
        knock2_played = false;
        return STATE_FOREST_PLAYING;
    }
    return STATE_NONE;
}

GameState process_forest_talking_game_over(InputState state) {
    // Play speech once at start
    static bool audio_played = false;
    if (!audio_played && audio.speak_gameover) {
        play(audio.speak_gameover);
        audio_played = true;
    }
    
    if (get_state_time() > 4.0) {
        audio_played = false;
        return STATE_CAVE_WAITING_BEFORE_TALKING;
    }
    return STATE_NONE;
}

GameState process_forest_win_talking(InputState state) {
    // Play speech once at start
    static bool audio_played = false;
    if (!audio_played && audio.speak_levelcompleted) {
        play(audio.speak_levelcompleted);
        audio_played = true;
    }
    
    // Hugo talks after winning forest
    if (get_state_time() > 3.0) {
        audio_played = false;
        // Transition to cave bonus game
        return STATE_CAVE_WAITING_BEFORE_TALKING;
    }
    return STATE_NONE;
}

// Process win state (legacy - redirects to win_talking)
GameState process_win(InputState state) {
    if (get_state_time() > 3.0) {
        // Transition to cave bonus game
        return STATE_CAVE_WAITING_BEFORE_TALKING;
    }
    return STATE_NONE;
}

// Process game over state
GameState process_game_over(InputState state) {
    if (get_state_time() > 5.0) {
        return STATE_END;
    }
    return STATE_NONE;
}
