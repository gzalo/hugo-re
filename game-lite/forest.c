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
    game_ctx.score = 0;
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
    if (textures.scoreboard) {
        SDL_Rect scoreboard = {0, 184, SCREEN_WIDTH, SCREEN_HEIGHT - 184};
        SDL_RenderCopy(renderer, textures.scoreboard, NULL, &scoreboard);
    } else {
        SDL_Rect scoreboard = {0, 184, SCREEN_WIDTH, SCREEN_HEIGHT - 184};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &scoreboard);
    }
    
    // Lives indicators
    for (int i = 0; i < game_ctx.lives; i++) {
        SDL_Rect life = {32 + i * 40, 188, 30, 30};
        if (textures.hugo_lives) {
            SDL_RenderCopy(renderer, textures.hugo_lives, NULL, &life);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
            SDL_RenderFillRect(renderer, &life);
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
        SDL_Rect src = {1 + xpos * 33, 1 + ypos * 34, 32, 33};
        SDL_Rect dst = {x_score + x_space * i, y_score, 24, 28};
        SDL_RenderCopy(renderer, textures.score_numbers, &src, &dst);
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
                    SDL_Rect cat = {(int)obstacle_pos - 8, 120, 32, 38};
                    if (textures.catapult[0]) {
                        int cat_frame = frame % 8;
                        SDL_RenderCopy(renderer, textures.catapult[cat_frame], NULL, &cat);
                    } else {
                        SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255);
                        SDL_RenderFillRect(renderer, &cat);
                    }
                }
                break;
            case OBS_TRAP:
                {
                    SDL_Rect trap = {(int)obstacle_pos - 8, 152, 32, 24};
                    if (textures.trap[0]) {
                        int trap_frame = frame % 6;
                        SDL_RenderCopy(renderer, textures.trap[trap_frame], NULL, &trap);
                    } else {
                        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
                        SDL_RenderFillRect(renderer, &trap);
                    }
                }
                break;
            case OBS_ROCK:
                {
                    double fract = game_ctx.parallax_pos - floor(game_ctx.parallax_pos);
                    int bounce = (int)(sin(fract * 2 * M_PI) * 15);
                    SDL_Rect rock = {(int)obstacle_pos - 15, 120 - bounce, 30, 30};
                    if (textures.rock[0]) {
                        int rock_frame = frame % 8;
                        SDL_RenderCopy(renderer, textures.rock[rock_frame], NULL, &rock);
                    } else {
                        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
                        SDL_RenderFillRect(renderer, &rock);
                    }
                }
                break;
            case OBS_TREE:
                {
                    // Render tree trunk if available
                    if (textures.lone_tree) {
                        SDL_Rect trunk = {(int)obstacle_pos - 52, -40, 104, 198};
                        SDL_RenderCopy(renderer, textures.lone_tree, NULL, &trunk);
                    }
                    // Render swinging branch
                    SDL_Rect tree = {(int)obstacle_pos, 62, 16, 96};
                    if (textures.tree[0]) {
                        int tree_frame = frame % 7;
                        SDL_RenderCopy(renderer, textures.tree[tree_frame], NULL, &tree);
                    } else {
                        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
                        SDL_RenderFillRect(renderer, &tree);
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
        
        SDL_Rect sack = {(int)sack_pos - 16, 32, 32, 32};
        int sack_frame = frame % 4;
        SDL_RenderCopy(renderer, textures.sack[sack_frame], NULL, &sack);
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
        
        SDL_Rect hugo = {HUGO_X_POS, hugo_y, 40, 60};
        if (textures.hugo_jump[0]) {
            int jump_frame = frame % 3;
            SDL_RenderCopy(renderer, textures.hugo_jump[jump_frame], NULL, &hugo);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 150, 0, 255);
            SDL_RenderFillRect(renderer, &hugo);
        }
    } else if (game_ctx.arrow_down_focus && game_ctx.hugo_crawling_time >= 0) {
        // Crawling animation
        hugo_y = 105;
        SDL_Rect hugo = {HUGO_X_POS, hugo_y, 40, 45};
        if (textures.hugo_crawl[0]) {
            int crawl_frame = frame % 8;
            SDL_RenderCopy(renderer, textures.hugo_crawl[crawl_frame], NULL, &hugo);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 150, 0, 255);
            SDL_RenderFillRect(renderer, &hugo);
        }
    } else {
        // Normal running animation
        SDL_Rect hugo = {HUGO_X_POS, hugo_y, 40, 60};
        if (textures.hugo_side[0]) {
            int side_frame = frame % 8;
            SDL_RenderCopy(renderer, textures.hugo_side[side_frame], NULL, &hugo);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 150, 0, 255);
            SDL_RenderFillRect(renderer, &hugo);
        }
    }
}

// Render control indicators
void render_controls() {
    SDL_Rect up_arrow = {256, 16, 40, 30};
    SDL_Rect down_arrow = {256, 54, 40, 30};
    
    // Use actual arrow textures if available
    // arrows[0] = up normal, arrows[1] = up pressed
    // arrows[2] = down normal, arrows[3] = down pressed
    if (game_ctx.arrow_up_focus) {
        SDL_RenderCopy(renderer, textures.arrows[1], NULL, &up_arrow);
    } else {
        SDL_RenderCopy(renderer, textures.arrows[0], NULL, &up_arrow);
    }
        
    if (game_ctx.arrow_down_focus) {
        SDL_RenderCopy(renderer, textures.arrows[3], NULL, &down_arrow);
    } else {
        SDL_RenderCopy(renderer, textures.arrows[2], NULL, &down_arrow);
    }
}


// Render background (simplified)
void render_background() {
    // Render gradient or solid sky
    if (textures.bg_gradient) {
        SDL_Rect sky = {0, 0, SCREEN_WIDTH, 158};
        SDL_RenderCopy(renderer, textures.bg_gradient, NULL, &sky);
    } else {
        SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
        SDL_Rect sky = {0, 0, SCREEN_WIDTH, 158};
        SDL_RenderFillRect(renderer, &sky);
    }
    
    // Render parallax background layers (hills, trees) if available
    if (textures.bg_hillsday) {
        // Simple parallax - move slower than foreground
        int hills_offset = (int)(game_ctx.parallax_pos * 6) % 320;
        SDL_Rect hills_rect = {-hills_offset, 0, 320, 158};
        SDL_RenderCopy(renderer, textures.bg_hillsday, NULL, &hills_rect);
        // Render second copy for seamless scroll
        SDL_Rect hills_rect2 = {320 - hills_offset, 0, 320, 158};
        SDL_RenderCopy(renderer, textures.bg_hillsday, NULL, &hills_rect2);
    }
    
    if (textures.bg_trees) {
        int trees_offset = (int)(game_ctx.parallax_pos * 12) % 320;
        SDL_Rect trees_rect = {-trees_offset, -24, 320, 182};
        SDL_RenderCopy(renderer, textures.bg_trees, NULL, &trees_rect);
        SDL_Rect trees_rect2 = {320 - trees_offset, -24, 320, 182};
        SDL_RenderCopy(renderer, textures.bg_trees, NULL, &trees_rect2);
    }
    
    // Render ground layer
    int ground_offset = (int)(game_ctx.parallax_pos * FOREST_GROUND_SPEED) % 320;
    for (int i = 0; i < 5; i++) {
        SDL_Rect ground_rect = {i * 320 - ground_offset, 158, 320, 82};
        SDL_RenderCopy(renderer, textures.bg_ground, NULL, &ground_rect);
    }
        
    // Render grass layer
    int grass_offset = (int)(game_ctx.parallax_pos * 30) % 320;
    for (int i = 0; i < 12; i++) {
        SDL_Rect grass_rect = {i * 27 - grass_offset, 172, 27, 68};
        SDL_RenderCopy(renderer, textures.grass, NULL, &grass_rect);
    }
}


// Render intro animation
void render_forest_wait_intro() {
    SDL_SetRenderDrawColor(renderer, 50, 100, 50, 255);
    SDL_RenderClear(renderer);
    
    int frame = get_frame_index() % 16;
    SDL_Rect hugo = {128, -16, 64, 256};
    SDL_RenderCopy(renderer, textures.hugo_telllives[frame], NULL, &hugo);
    
    // Lives indicator
    for (int i = 0; i < game_ctx.lives; i++) {
        SDL_Rect life = {32 + i * 40, 188, 30, 30};
        if (textures.hugo_lives) {
            SDL_RenderCopy(renderer, textures.hugo_lives, NULL, &life);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
            SDL_RenderFillRect(renderer, &life);
        }
    }
    
    SDL_RenderPresent(renderer);
}

// Render playing state
void render_forest_playing() {
    render_background();
    render_obstacles();
    render_sacks();
    render_hugo();
    render_controls();
    render_hud();
    
    SDL_RenderPresent(renderer);
}

// Forest hurt state rendering functions
void render_forest_branch_animation() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hugohitlog.frames && textures.hugohitlog.frame_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.hugohitlog.frame_count) frame = textures.hugohitlog.frame_count - 1;
        if (textures.hugohitlog.frames[frame]) {
            SDL_RenderCopy(renderer, textures.hugohitlog.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_branch_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hugohitlog_talk.frames && textures.hugohitlog_talk.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.hugohitlog_talk.frame_count) frame = textures.hugohitlog_talk.frame_count - 1;
        if (textures.hugohitlog_talk.frames[frame]) {
            SDL_RenderCopy(renderer, textures.hugohitlog_talk.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_flying_start() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.catapult_fly.frames && textures.catapult_fly.frame_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.catapult_fly.frame_count) frame = textures.catapult_fly.frame_count - 1;
        if (textures.catapult_fly.frames[frame]) {
            SDL_RenderCopy(renderer, textures.catapult_fly.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_flying_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.catapult_airtalk.frames && textures.catapult_airtalk.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.catapult_airtalk.frame_count) frame = textures.catapult_airtalk.frame_count - 1;
        if (textures.catapult_airtalk.frames[frame]) {
            SDL_RenderCopy(renderer, textures.catapult_airtalk.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_flying_falling() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.catapult_fall.frames && textures.catapult_fall.frame_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.catapult_fall.frame_count) frame = textures.catapult_fall.frame_count - 1;
        if (textures.catapult_fall.frames[frame]) {
            SDL_RenderCopy(renderer, textures.catapult_fall.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_flying_falling_hang_animation() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.catapult_hang.frames && textures.catapult_hang.frame_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.catapult_hang.frame_count) frame = textures.catapult_hang.frame_count - 1;
        if (textures.catapult_hang.frames[frame]) {
            SDL_RenderCopy(renderer, textures.catapult_hang.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_flying_falling_hang_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render static hang frame as background
    if (textures.catapult_hang.frames && textures.catapult_hang.frame_count > 12) {
        if (textures.catapult_hang.frames[12]) {
            SDL_RenderCopy(renderer, textures.catapult_hang.frames[12], NULL, NULL);
        }
    }
    
    // Render talking mouth animation on top
    if (textures.catapult_hangspeak.frames && textures.catapult_hangspeak.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.catapult_hangspeak.frame_count) frame = textures.catapult_hangspeak.frame_count - 1;
        if (textures.catapult_hangspeak.frames[frame]) {
            SDL_Rect mouth_pos = {115, 117, 0, 0};
            SDL_QueryTexture(textures.catapult_hangspeak.frames[frame], NULL, NULL, &mouth_pos.w, &mouth_pos.h);
            SDL_RenderCopy(renderer, textures.catapult_hangspeak.frames[frame], NULL, &mouth_pos);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_rock_animation() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hugo_lookrock.frames && textures.hugo_lookrock.frame_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.hugo_lookrock.frame_count) frame = textures.hugo_lookrock.frame_count - 1;
        if (textures.hugo_lookrock.frames[frame]) {
            SDL_RenderCopy(renderer, textures.hugo_lookrock.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_rock_hit_animation() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hit_rock.frames && textures.hit_rock.frame_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.hit_rock.frame_count) frame = textures.hit_rock.frame_count - 1;
        if (textures.hit_rock.frames[frame]) {
            SDL_RenderCopy(renderer, textures.hit_rock.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_rock_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hit_rock_sync.frames && textures.hit_rock_sync.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.hit_rock_sync.frame_count) frame = textures.hit_rock_sync.frame_count - 1;
        if (textures.hit_rock_sync.frames[frame]) {
            SDL_RenderCopy(renderer, textures.hit_rock_sync.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_trap_animation() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hugo_traphurt.frames && textures.hugo_traphurt.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.hugo_traphurt.frame_count) frame = textures.hugo_traphurt.frame_count - 1;
        if (textures.hugo_traphurt.frames[frame]) {
            SDL_RenderCopy(renderer, textures.hugo_traphurt.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_trap_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hugo_traptalk.frames && textures.hugo_traptalk.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.hugo_traptalk.frame_count) frame = textures.hugo_traptalk.frame_count - 1;
        if (textures.hugo_traptalk.frames[frame]) {
            SDL_RenderCopy(renderer, textures.hugo_traptalk.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_scylla_button() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Scylla button is shown in multiplayer mode - not typically used in single player
    // Just render a blank screen for now
    
    SDL_RenderPresent(renderer);
}

void render_forest_talking_after_hurt() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render Hugo telllives animation
    if (textures.hugo_telllives[0]) {
        int frame = get_frame_index();
        if (frame >= 16) frame = 15;
        SDL_Rect hugo = {128, -16, 64, 256};
        SDL_RenderCopy(renderer, textures.hugo_telllives[frame], NULL, &hugo);
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_talking_game_over() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render Hugo telllives animation for game over
    if (textures.hugo_telllives[0]) {
        int frame = get_frame_index();
        if (frame >= 16) frame = 15;
        SDL_Rect hugo = {128, -16, 64, 256};
        SDL_RenderCopy(renderer, textures.hugo_telllives[frame], NULL, &hugo);
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_win_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render Hugo telllives animation for win
    if (textures.hugo_telllives[0]) {
        int frame = get_frame_index();
        if (frame >= 16) frame = 15;
        SDL_Rect hugo = {128, -16, 64, 256};
        SDL_RenderCopy(renderer, textures.hugo_telllives[frame], NULL, &hugo);
    }
    
    SDL_RenderPresent(renderer);
}

// Cave game rendering functions
void render_cave_waiting_before_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.cave_talks.frames && textures.cave_talks.frame_count > 12) {
        // Show frame 12 (last frame of talks before climbing)
        if (textures.cave_talks.frames[12]) {
            SDL_RenderCopy(renderer, textures.cave_talks.frames[12], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_talking_before_climb() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.cave_talks.frames && textures.cave_talks.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_talks.frame_count) frame = textures.cave_talks.frame_count - 1;
        if (textures.cave_talks.frames[frame]) {
            SDL_RenderCopy(renderer, textures.cave_talks.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_climbing() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.cave_climbs.frames && textures.cave_climbs.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_climbs.frame_count) frame = textures.cave_climbs.frame_count - 1;
        if (textures.cave_climbs.frames[frame]) {
            SDL_RenderCopy(renderer, textures.cave_climbs.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_waiting_input() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Show last frame of climbing animation
    if (textures.cave_climbs.frames && textures.cave_climbs.frame_count > 0) {
        int last_frame = textures.cave_climbs.frame_count - 1;
        if (textures.cave_climbs.frames[last_frame]) {
            SDL_RenderCopy(renderer, textures.cave_climbs.frames[last_frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_going_rope() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    SDL_Texture** rope_animation = NULL;
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
            SDL_RenderCopy(renderer, rope_animation[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_lost() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    SDL_Texture** puff_animation = NULL;
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
            SDL_RenderCopy(renderer, puff_animation[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_lost_spring() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.cave_hugo_spring.frames && textures.cave_hugo_spring.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_hugo_spring.frame_count) frame = textures.cave_hugo_spring.frame_count - 1;
        if (textures.cave_hugo_spring.frames[frame]) {
            SDL_RenderCopy(renderer, textures.cave_hugo_spring.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_scylla_lost() {
    // Scylla loses (Hugo wins)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    SDL_Texture** scylla_animation = NULL;
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
            SDL_RenderCopy(renderer, scylla_animation[frame], NULL, NULL);
        }
    }
    
    // Render Hugo sprite on top
    if (textures.cave_hugo_sprite) {
        int hugo_positions[3][2] = {{25, 105}, {97, 100}, {172, 102}};
        SDL_Rect hugo_rect;
        SDL_QueryTexture(textures.cave_hugo_sprite, NULL, NULL, &hugo_rect.w, &hugo_rect.h);
        hugo_rect.x = hugo_positions[game_ctx.cave_selected_rope][0];
        hugo_rect.y = hugo_positions[game_ctx.cave_selected_rope][1];
        SDL_RenderCopy(renderer, textures.cave_hugo_sprite, NULL, &hugo_rect);
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_scylla_spring() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.cave_scylla_spring.frames && textures.cave_scylla_spring.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_scylla_spring.frame_count) frame = textures.cave_scylla_spring.frame_count - 1;
        if (textures.cave_scylla_spring.frames[frame]) {
            SDL_RenderCopy(renderer, textures.cave_scylla_spring.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_family_cage_opens() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.cave_family_cage.frames && textures.cave_family_cage.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_family_cage.frame_count) frame = textures.cave_family_cage.frame_count - 1;
        if (textures.cave_family_cage.frames[frame]) {
            SDL_RenderCopy(renderer, textures.cave_family_cage.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_family_happy() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.cave_happy.frames && textures.cave_happy.frame_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_happy.frame_count) frame = textures.cave_happy.frame_count - 1;
        if (textures.cave_happy.frames[frame]) {
            SDL_RenderCopy(renderer, textures.cave_happy.frames[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
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

// Process wait intro state
GameState process_forest_wait_intro(InputState state) {
    // Play intro speech once at start
    static bool intro_played = false;
    if (!intro_played && audio.speak_start) {
        Mix_PlayChannel(-1, audio.speak_start, 0);
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
                if (audio.sfx_hugo_launch) Mix_PlayChannel(-1, audio.sfx_hugo_launch, 0);
                if (audio.sfx_catapult_eject) Mix_PlayChannel(-1, audio.sfx_catapult_eject, 0);
                game_ctx.obstacles[current_pos] = OBS_NONE;
                return STATE_FOREST_FLYING_START;
            } else if (obs == OBS_TRAP && !game_ctx.arrow_up_focus) {
                // Trap hit
                if (audio.sfx_hugo_hittrap) Mix_PlayChannel(-1, audio.sfx_hugo_hittrap, 0);
                game_ctx.obstacles[current_pos] = OBS_NONE;
                return STATE_FOREST_TRAP_ANIMATION;
            } else if (obs == OBS_ROCK && !game_ctx.arrow_up_focus) {
                // Rock hit
                if (audio.sfx_hugo_hitlog) Mix_PlayChannel(-1, audio.sfx_hugo_hitlog, 0);
                game_ctx.obstacles[current_pos] = OBS_NONE;
                return STATE_FOREST_ROCK_ANIMATION;
            } else if (obs == OBS_TREE && !game_ctx.arrow_down_focus) {
                // Branch hit (need to duck to avoid)
                if (audio.sfx_hugo_hitlog) Mix_PlayChannel(-1, audio.sfx_hugo_hitlog, 0);
                game_ctx.obstacles[current_pos] = OBS_NONE;
                return STATE_FOREST_BRANCH_ANIMATION;
            } else if (obs == OBS_TREE && game_ctx.arrow_down_focus) {
                // Ducked under tree successfully
                if (audio.sfx_tree_swush) Mix_PlayChannel(-1, audio.sfx_tree_swush, 0);
            }
        }
        
        // Check sack collection
        if (game_ctx.arrow_up_focus && game_ctx.sacks[current_pos] != 0) {
            if (game_ctx.sacks[current_pos] == 1) {
                game_ctx.score += 100;
                if (audio.sfx_sack_normal) Mix_PlayChannel(-1, audio.sfx_sack_normal, 0);
            } else {
                game_ctx.score += 250;
                if (audio.sfx_sack_bonus) Mix_PlayChannel(-1, audio.sfx_sack_bonus, 0);
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
        Mix_PlayChannel(-1, audio.sfx_birds, 0);
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
        Mix_PlayChannel(-1, audio.speak_hitlog, 0);
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
        if (audio.speak_catapult_up) Mix_PlayChannel(-1, audio.speak_catapult_up, 0);
        audio_played = true;
    }
    
    // Play crash sound at 2.7 seconds
    static bool crash_played = false;
    if (!crash_played && get_state_time() > 2.7) {
        if (audio.sfx_hugo_screenklir) Mix_PlayChannel(-1, audio.sfx_hugo_screenklir, 0);
        if (audio.speak_catapult_hit) Mix_PlayChannel(-1, audio.speak_catapult_hit, 0);
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
        Mix_PlayChannel(-1, audio.speak_catapult_talktop, 0);
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
        if (audio.speak_catapult_down) Mix_PlayChannel(-1, audio.speak_catapult_down, 0);
        if (audio.sfx_hugo_crash) Mix_PlayChannel(-1, audio.sfx_hugo_crash, 0);
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
        Mix_PlayChannel(-1, audio.sfx_hugo_hangstart, 0);
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
        if (audio.speak_catapult_hang) Mix_PlayChannel(-1, audio.speak_catapult_hang, 0);
        if (audio.sfx_hugo_hang) Mix_PlayChannel(-1, audio.sfx_hugo_hang, 0);
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
        Mix_PlayChannel(-1, audio.speak_rock, 0);
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
        Mix_PlayChannel(-1, audio.speak_trap, 0);
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
        Mix_PlayChannel(-1, audio.sfx_lightning_warning, 0);
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
        if (audio.sfx_hugo_knock) Mix_PlayChannel(-1, audio.sfx_hugo_knock, 0);
        if (game_ctx.lives == 1 && audio.speak_lastlife) {
            Mix_PlayChannel(-1, audio.speak_lastlife, 0);
        } else if (audio.speak_dieonce) {
            Mix_PlayChannel(-1, audio.speak_dieonce, 0);
        }
        audio_played = true;
    }
    
    // Play knock again at 0.5 seconds
    static bool knock2_played = false;
    if (!knock2_played && get_state_time() > 0.5 && audio.sfx_hugo_knock) {
        Mix_PlayChannel(-1, audio.sfx_hugo_knock, 0);
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
        Mix_PlayChannel(-1, audio.speak_gameover, 0);
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
        Mix_PlayChannel(-1, audio.speak_levelcompleted, 0);
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
