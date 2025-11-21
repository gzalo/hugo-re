#include "common.h"
#include <stdbool.h>
#include <stdio.h>
#include "state.h"
#include <math.h>
#include <stdlib.h>   // rand, srand
#include <time.h>     // time

#include "forest.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static ForestContext game_ctx = {0};
static ForestState current_forest_state = STATE_FOREST_WAIT_INTRO;
static double current_state_time = 0.0;

#define FOREST_BG_SPEED_MULTIPLIER 1.0

static double new_mod(double a, double b) {
    double res = fmod(a, b);
    if (res == 0.0) return res;
    return (a < 0.0) ? res - b : res;
}


// -----------------------------------------------------------------------------
// Obstacle / sack generation
// -----------------------------------------------------------------------------

void generate_obstacles() {
    // Match Python: empty 65%, others 8.75% each (35%/4 = 8.75%)
    // 0 = none, 1 = catapult, 2 = trap, 3 = rock, 4 = tree
    // Using 10000 scale for more precision: 6500, 875, 875, 875, 875
    for (int i = 0; i < FOREST_MAX_TIME; i++) {
        int r = rand() % 10000;
        if (r < 6500) {
            game_ctx.obstacles[i] = OBS_NONE;
        } else if (r < 6500 + 875) {        // catapult: 8.75%
            game_ctx.obstacles[i] = OBS_CATAPULT;
        } else if (r < 6500 + 1750) {       // trap: 8.75%
            game_ctx.obstacles[i] = OBS_TRAP;
        } else if (r < 6500 + 2625) {       // rock: 8.75%
            game_ctx.obstacles[i] = OBS_ROCK;
        } else {                            // tree: 8.75%
            game_ctx.obstacles[i] = OBS_TREE;
        }
    }

    // Clear first few positions
    if (FOREST_MAX_TIME > 0) game_ctx.obstacles[0] = OBS_NONE;
    if (FOREST_MAX_TIME > 1) game_ctx.obstacles[1] = OBS_NONE;
    if (FOREST_MAX_TIME > 2) game_ctx.obstacles[2] = OBS_NONE;
    if (FOREST_MAX_TIME > 3) game_ctx.obstacles[3] = OBS_NONE;

    // Ensure no consecutive obstacles
    for (int i = 0; i < FOREST_MAX_TIME - 1; i++) {
        if (game_ctx.obstacles[i] != OBS_NONE) {
            game_ctx.obstacles[i + 1] = OBS_NONE;
        }
    }
}

void generate_sacks() {
    // Match Python probabilities exactly:
    // empty 70%, normal 21% (other_prob = 0.3 * 0.7 = 0.21), gold 9% (other_prob_high = 0.3 * 0.3 = 0.09)
    for (int i = 0; i < FOREST_MAX_TIME; i++) {
        int r = rand() % 100;
        if (r < 70) {
            game_ctx.sacks[i] = 0; // No sack (70%)
        } else if (r < 91) {  // 70 + 21 = 91
            game_ctx.sacks[i] = 1; // Normal sack (21%)
        } else {
            game_ctx.sacks[i] = 2; // Gold sack (9%)
        }
    }
}

void generate_leaves() {
    for (int i = 0; i < FOREST_MAX_TIME; i++) {
        int r = rand() % 100;
        game_ctx.leaves[i] = (r < 50) ? 1 : 2;
    }

    for (int i = 0; i < FOREST_MAX_TIME - 1; i++) {
        if (game_ctx.leaves[i] == 2) {
            game_ctx.leaves[i + 1] = 0;
        }
    }
}

void init_game_context() {
    // Optional: seed RNG once (if not done elsewhere)
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

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
    generate_leaves();
}

// ---------------- HUD / BOTTOM ----------------

// Helper to get spritesheet area (matches Python get_spritesheet_area)
typedef struct {
    int x;
    int y;
    int width;
    int height;
} SpriteSheetArea;


static SpriteSheetArea get_spritesheet_area(int value) {
    int width = 32;
    int height = 33;
    int xpos = value % 5;
    int ypos = value / 5;
    
    SpriteSheetArea area;
    area.x = 1 + xpos * (width + 1);
    area.y = 1 + ypos * (height + 1);
    area.width = width;
    area.height = height;
    return area;
}

void render_forest_bottom() {
    render(textures.scoreboard, 0, 184);

    // Lives indicators
    for (int i = 0; i < game_ctx.lives; i++) {
        if (textures.hugo_lives) {
            render(textures.hugo_lives, 32 + i * 40, 188);
        }
    }

    // Score digits (spritesheet 5x2; 32x33 cells with 1px spacing)
    int x_score = 200;
    int y_score = 194;
    int x_space = 24;
    int thousands = game_ctx.score / 1000;
    int hundreds = (game_ctx.score % 1000) / 100;
    int tens     = (game_ctx.score % 100) / 10;
    int ones     = game_ctx.score % 10;

    int vals[4] = { thousands, hundreds, tens, ones };
    for (int i = 0; i < 4; i++) {
        SpriteSheetArea area = get_spritesheet_area(vals[i]);
        render_subtexture(textures.score_numbers, area.x, area.y, area.width, area.height,
                          x_score + x_space * i, y_score);
    }
}

// ---------------- RENDERING: BACKGROUND / OBSTACLES / SACKS / HUGO / CONTROLS ----------------

void render_obstacles() {
    int frame = get_frame_index(current_state_time);
    double fract = game_ctx.parallax_pos - floor(game_ctx.parallax_pos);

    for (int i = 0; i < FOREST_MAX_TIME; i++) {
        double obstacle_pos = (i - game_ctx.parallax_pos) * FOREST_GROUND_SPEED;

        if (obstacle_pos < -50 || obstacle_pos > SCREEN_WIDTH + 50) {
            continue;
        }

        switch (game_ctx.obstacles[i]) {
        case OBS_CATAPULT: {
            int idx = frame % (sizeof(textures.catapult)/sizeof(textures.catapult[0]));
            int dy[8] = { 45, 43, 39, 34, 29, 22, 14, 1 };
            int y = 112 + dy[idx % 8];
            Texture *t = textures.catapult[idx];
            if (t) render(t, (int)(obstacle_pos - 8), y);
            break;
        }
        case OBS_TRAP: {
            int idx = frame % (sizeof(textures.trap)/sizeof(textures.trap[0]));
            int dy[6] = { 176, 173, 169, 165, 176, 176 };
            int y = dy[idx % 6] - 24;
            Texture *t = textures.trap[idx];
            if (t) render(t, (int)(obstacle_pos - 8), y);
            break;
        }
        case OBS_ROCK: {
            int idx = frame % (sizeof(textures.rock)/sizeof(textures.rock[0]));
            double offset = sin(fract * (2.0 * M_PI)) * 15.0;
            Texture *t = textures.rock[idx];
            if (t) render(t, (int)(obstacle_pos - offset), 120);
            break;
        }
        case OBS_TREE: {
            int idx = frame % (sizeof(textures.tree)/sizeof(textures.tree[0]));
            Texture *lone = textures.lone_tree;
            Texture *swing = textures.tree[idx];
            if (lone)  render(lone,  (int)(obstacle_pos - 52), -40);
            if (swing) render(swing, (int)obstacle_pos, 52 + 10);
            break;
        }
        default:
            break;
        }
    }
}

void render_sacks() {
    (void)get_frame_index(current_state_time); // frame not used but kept for parity

    for (int i = 0; i < FOREST_MAX_TIME; i++) {
        if (game_ctx.sacks[i] == 0) continue;

        double sack_pos = (i - game_ctx.parallax_pos) * FOREST_GROUND_SPEED;
        if (sack_pos < -50 || sack_pos > SCREEN_WIDTH + 50) {
            continue;
        }

        Texture *t = textures.sack[0];
        if (t) render(t, (int)(sack_pos - 16), 32);
    }
}

void render_leaves() {
    for (int i = 0; i < FOREST_MAX_TIME; i++) {
        if (game_ctx.leaves[i] == 0) continue;

        double leave_pos = (i - game_ctx.parallax_pos) * FOREST_GROUND_SPEED;
        if (leave_pos < -50 || leave_pos > SCREEN_WIDTH + 50) {
            continue;
        }

        Texture *tex = NULL;
        if (game_ctx.leaves[i] == 1) {
            tex = textures.leaves2;
        } else if (game_ctx.leaves[i] == 2) {
            tex = textures.leaves1;
        }

        if (tex) {
            render(tex, (int)(leave_pos - 16), -10);
        }
    }
}

void render_hugo() {
    int frame = get_frame_index(current_state_time);
    int hugo_x = HUGO_X_POS;
    double now = get_game_time();

    if (game_ctx.arrow_up_focus && game_ctx.hugo_jumping_time >= 0) {
        double dt = (now - game_ctx.hugo_jumping_time) / 0.75;
        double dy = -250 * dt * dt + 250 * dt - 22.5;
        int y = (int)(40 - dy);
        int idx = frame % 3;
        Texture *t = textures.hugo_jump[idx];
        if (t) render(t, hugo_x, y);
    } else if (game_ctx.arrow_down_focus && game_ctx.hugo_crawling_time >= 0) {
        int y = 105;
        int idx = frame % 8;
        Texture *t = textures.hugo_crawl[idx];
        if (t) render(t, hugo_x, y);
    } else {
        int y = 90;
        int idx = frame % 8;
        Texture *t = textures.hugo_side[idx];
        if (t) render(t, hugo_x, y);
    }
}

void render_controls() {
    // arrows[0] up normal, [1] up pressed, [2] down normal, [3] down pressed
    if (game_ctx.arrow_up_focus) {
        render(textures.arrows[1], 256, 17);
    } else {
        render(textures.arrows[0], 256 + 2, 16 + 3);
    }

    if (game_ctx.arrow_down_focus) {
        render(textures.arrows[3], 256, 54);
    } else {
        render(textures.arrows[2], 256 + 2, 54 + 2);
    }
}

// Render background (match Python parallax as close as possible)
void render_forest_background() {
    double hills_speed = 6.0 * FOREST_BG_SPEED_MULTIPLIER;
    double trees_speed = 12.0 * FOREST_BG_SPEED_MULTIPLIER;
    double grass_speed = 30.0 * FOREST_BG_SPEED_MULTIPLIER;

    // Query texture widths each frame so scrolling repeats with the real asset size
    int hills_width  = query_texture_width(textures.bg_hillsday);
    int trees_width  = query_texture_width(textures.bg_trees);
    int grass_width  = query_texture_width(textures.grass);
    int ground_width = query_texture_width(textures.bg_ground);

    double p = game_ctx.parallax_pos;

    int hills_x   = (int)new_mod(-p * hills_speed,          (double)hills_width);
    int trees_x   = (int)new_mod(-p * trees_speed,          (double)trees_width);
    int grass_x0  = (int)new_mod(-p * grass_speed,          (double)grass_width);
    int ground_x0 = (int)new_mod(-p * FOREST_GROUND_SPEED,  (double)ground_width);

    int mountain_x = 320 - 96 - (int)((p - FOREST_MAX_TIME) * FOREST_GROUND_SPEED);

    // Gradient sky
    if (textures.bg_gradient) {
        render(textures.bg_gradient, 0, 0);
    }

    // Hills
    if (textures.bg_hillsday) {
        render(textures.bg_hillsday, hills_x, 0);
        render(textures.bg_hillsday, hills_x + hills_width, 0);
    }

    // Trees
    if (textures.bg_trees) {
        render(textures.bg_trees, trees_x, -24);
        render(textures.bg_trees, trees_x + trees_width, -24);
    }

    // Ground tiles
    if (textures.bg_ground) {
        for (int i = 0; i < 5; i++) {
            int x = ground_x0 + i * ground_width;
            render(textures.bg_ground, x, 158);
        }
    }

    // Grass tiles
    if (textures.grass) {
        for (int i = 0; i < 12; i++) {
            int x = grass_x0 + i * grass_width;
            render(textures.grass, x, 172);
        }
    }

    // End mountain
    if (textures.end_mountain) {
        render(textures.end_mountain, mountain_x, -16);
    }
}

void render_forest_playing() {
    render_forest_background();
    render_obstacles();
    render_sacks();
    render_leaves();
    render_hugo();
    render_controls();
    render_forest_bottom();
}

void on_enter_forest_playing() {
    init_game_context();
    game_ctx.arrow_up_focus = false;
    game_ctx.arrow_down_focus = false;
    game_ctx.hugo_jumping_time = -1;
    game_ctx.hugo_crawling_time = -1;
    game_ctx.last_time = get_game_time();
    game_ctx.old_second = -1;
}

// ---------------- HURT / TALKING RENDERING ----------------

void render_forest_branch_animation() {
    if (textures.hugohitlog.frames && textures.hugohitlog.frame_count > 0) {
        int frame = get_frame_index_fast(current_state_time);
        if (frame >= textures.hugohitlog.frame_count) frame = textures.hugohitlog.frame_count - 1;
        Texture *t = textures.hugohitlog.frames[frame];
        if (t) render(t, 0, 0);
    }
}

void render_forest_branch_talking() {
    int frame = get_frame_index(current_state_time);
    
    // Use sync_hitlog with hugohitlog_talk animation
    if (textures.hugohitlog_talk.frames && textures.hugohitlog_talk.frame_count > 0 && 
        textures.sync_hitlog && textures.sync_hitlog_count > 0) {
        int sync_idx = frame;
        if (sync_idx >= textures.sync_hitlog_count) sync_idx = textures.sync_hitlog_count - 1;
        
        int anim_frame = textures.sync_hitlog[sync_idx] - 1;
        if (anim_frame < 0) anim_frame = 0;
        if (anim_frame >= textures.hugohitlog_talk.frame_count) anim_frame = textures.hugohitlog_talk.frame_count - 1;
        
        Texture *t = textures.hugohitlog_talk.frames[anim_frame];
        if (t) render(t, 0, 0);
    } else if (textures.hugohitlog_talk.frames && textures.hugohitlog_talk.frame_count > 0) {
        // Fallback to frame-based rendering
        if (frame >= textures.hugohitlog_talk.frame_count) frame = textures.hugohitlog_talk.frame_count - 1;
        Texture *t = textures.hugohitlog_talk.frames[frame];
        if (t) render(t, 0, 0);
    }
}

void render_forest_flying_start() {
    if (textures.catapult_fly.frames && textures.catapult_fly.frame_count > 0) {
        int frame = get_frame_index_fast(current_state_time);
        if (frame >= textures.catapult_fly.frame_count) frame = textures.catapult_fly.frame_count - 1;
        Texture *t = textures.catapult_fly.frames[frame];
        if (t) render(t, 0, 0);
    }
}

void render_forest_flying_talking() {
    int frame = get_frame_index(current_state_time);
    
    // Use sync_catapult_talktop with catapult_airtalk animation
    if (textures.catapult_airtalk.frames && textures.catapult_airtalk.frame_count > 0 && 
        textures.sync_catapult_talktop && textures.sync_catapult_talktop_count > 0) {
        int sync_idx = frame;
        if (sync_idx >= textures.sync_catapult_talktop_count) sync_idx = textures.sync_catapult_talktop_count - 1;
        
        int anim_frame = textures.sync_catapult_talktop[sync_idx] - 1;
        if (anim_frame < 0) anim_frame = 0;
        if (anim_frame >= textures.catapult_airtalk.frame_count) anim_frame = textures.catapult_airtalk.frame_count - 1;
        
        Texture *t = textures.catapult_airtalk.frames[anim_frame];
        if (t) render(t, 0, 0);
    } else if (textures.catapult_airtalk.frames && textures.catapult_airtalk.frame_count > 0) {
        // Fallback to frame-based rendering
        if (frame >= textures.catapult_airtalk.frame_count) frame = textures.catapult_airtalk.frame_count - 1;
        Texture *t = textures.catapult_airtalk.frames[frame];
        if (t) render(t, 0, 0);
    }
}

void render_forest_flying_falling() {
    if (textures.catapult_fall.frames && textures.catapult_fall.frame_count > 0) {
        int frame = get_frame_index_fast(current_state_time);
        if (frame >= textures.catapult_fall.frame_count) frame = textures.catapult_fall.frame_count - 1;
        Texture *t = textures.catapult_fall.frames[frame];
        if (t) render(t, 0, 0);
    }
}

void render_forest_flying_falling_hang_animation() {
    if (textures.catapult_hang.frames && textures.catapult_hang.frame_count > 0) {
        int frame = get_frame_index_fast(current_state_time);
        if (frame >= textures.catapult_hang.frame_count) frame = textures.catapult_hang.frame_count - 1;
        Texture *t = textures.catapult_hang.frames[frame];
        if (t) render(t, 0, 0);
    }
}

void render_forest_flying_falling_hang_talking() {
    int frame = get_frame_index(current_state_time);
    
    // Static hang frame 12 as background
    if (textures.catapult_hang.frames && textures.catapult_hang.frame_count > 12) {
        Texture *bg = textures.catapult_hang.frames[12];
        if (bg) render(bg, 0, 0);
    }

    // Use sync_catapult_hang with catapult_hangspeak mouth animation
    if (textures.catapult_hangspeak.frames && textures.catapult_hangspeak.frame_count > 0 && 
        textures.sync_catapult_hang && textures.sync_catapult_hang_count > 0) {
        int sync_idx = frame;
        if (sync_idx >= textures.sync_catapult_hang_count) sync_idx = textures.sync_catapult_hang_count - 1;
        
        int anim_frame = textures.sync_catapult_hang[sync_idx] - 1;
        if (anim_frame < 0) anim_frame = 0;
        if (anim_frame >= textures.catapult_hangspeak.frame_count) anim_frame = textures.catapult_hangspeak.frame_count - 1;
        
        Texture *mouth = textures.catapult_hangspeak.frames[anim_frame];
        if (mouth) render(mouth, 115, 117);
    } else if (textures.catapult_hangspeak.frames && textures.catapult_hangspeak.frame_count > 0) {
        // Fallback to frame-based mouth rendering
        if (frame >= textures.catapult_hangspeak.frame_count) frame = textures.catapult_hangspeak.frame_count - 1;
        Texture *mouth = textures.catapult_hangspeak.frames[frame];
        if (mouth) render(mouth, 115, 117);
    }
}

void render_forest_rock_animation() {
    if (textures.hugo_lookrock.frames && textures.hugo_lookrock.frame_count > 0) {
        int frame = get_frame_index_fast(current_state_time);
        if (frame >= textures.hugo_lookrock.frame_count) frame = textures.hugo_lookrock.frame_count - 1;
        Texture *t = textures.hugo_lookrock.frames[frame];
        if (t) render(t, 0, 0);
    }
}

void render_forest_rock_hit_animation() {
    if (textures.hit_rock.frames && textures.hit_rock.frame_count > 0) {
        int frame = get_frame_index_fast(current_state_time);
        if (frame >= textures.hit_rock.frame_count) frame = textures.hit_rock.frame_count - 1;
        Texture *t = textures.hit_rock.frames[frame];
        if (t) render(t, 0, 0);
    }
}

void render_forest_rock_talking() {
    int frame = get_frame_index(current_state_time);
    
    // Use sync_rock data if available, otherwise fall back to hit_rock_sync animation
    if (textures.hugo_telllives.frames && textures.hugo_telllives.frame_count > 0 && 
        textures.sync_rock && textures.sync_rock_count > 0) {
        int sync_idx = frame;
        if (sync_idx >= textures.sync_rock_count) sync_idx = textures.sync_rock_count - 1;
        
        int anim_frame = textures.sync_rock[sync_idx] - 1;
        if (anim_frame < 0) anim_frame = 0;
        if (anim_frame >= textures.hugo_telllives.frame_count) anim_frame = textures.hugo_telllives.frame_count - 1;
        
        Texture *t = textures.hugo_telllives.frames[anim_frame];
        if (t) render(t, 0, 0);
    } else if (textures.hit_rock_sync.frames && textures.hit_rock_sync.frame_count > 0) {
        // Fallback to hit_rock_sync animation
        if (frame >= textures.hit_rock_sync.frame_count) frame = textures.hit_rock_sync.frame_count - 1;
        Texture *t = textures.hit_rock_sync.frames[frame];
        if (t) render(t, 0, 0);
    }
}

void render_forest_trap_animation() {
    if (textures.hugo_traphurt.frames && textures.hugo_traphurt.frame_count > 0) {
        int frame = get_frame_index(current_state_time);
        if (frame >= textures.hugo_traphurt.frame_count) frame = textures.hugo_traphurt.frame_count - 1;
        Texture *t = textures.hugo_traphurt.frames[frame];
        if (t) render(t, 0, 0);
    }
}

void render_forest_trap_talking() {
    int frame = get_frame_index(current_state_time);
    
    // Use sync_trap data if available with hugo_telllives, otherwise use hugo_traptalk
    if (textures.hugo_telllives.frames && textures.hugo_telllives.frame_count > 0 && 
        textures.sync_trap && textures.sync_trap_count > 0) {
        int sync_idx = frame;
        if (sync_idx >= textures.sync_trap_count) sync_idx = textures.sync_trap_count - 1;
        
        int anim_frame = textures.sync_trap[sync_idx] - 1;
        if (anim_frame < 0) anim_frame = 0;
        if (anim_frame >= textures.hugo_telllives.frame_count) anim_frame = textures.hugo_telllives.frame_count - 1;
        
        Texture *t = textures.hugo_telllives.frames[anim_frame];
        if (t) render(t, 0, 0);
    } else if (textures.hugo_traptalk.frames && textures.hugo_traptalk.frame_count > 0) {
        // Fallback to hugo_traptalk animation
        if (frame >= textures.hugo_traptalk.frame_count) frame = textures.hugo_traptalk.frame_count - 1;
        Texture *t = textures.hugo_traptalk.frames[frame];
        if (t) render(t, 0, 0);
    }
}

void render_forest_scylla_button() {
    // Placeholder: GameTextures doesn't define scylla hand textures yet;
    // skip to avoid compilation error.
    // Implement when textures are added to GameTextures.
}

void render_forest_talking_after_hurt() {
    int frame = get_frame_index(current_state_time);
    
    // Use appropriate sync data based on lives remaining
    int* sync_data = (game_ctx.lives == 1) ? textures.sync_lastlife : textures.sync_dieonce;
    int sync_count = (game_ctx.lives == 1) ? textures.sync_lastlife_count : textures.sync_dieonce_count;
    
    // Get synced frame from hugo_telllives animation
    if (textures.hugo_telllives.frames && textures.hugo_telllives.frame_count > 0 && sync_data && sync_count > 0) {
        int sync_idx = frame;
        if (sync_idx >= sync_count) sync_idx = sync_count - 1;
        
        int anim_frame = sync_data[sync_idx] - 1;  // Sync indices are 1-based
        if (anim_frame < 0) anim_frame = 0;
        if (anim_frame >= textures.hugo_telllives.frame_count) anim_frame = textures.hugo_telllives.frame_count - 1;
        
        Texture *t = textures.hugo_telllives.frames[anim_frame];
        if (t) render(t, 128, -16);
    }

    // Hand animation
    if (frame < 8 && frame % 4 == 0) {
        if (textures.hugo_hand2) render(textures.hugo_hand2, 96, 78);
    } else {
        if (textures.hugo_hand1) render(textures.hugo_hand1, 96, 83);
    }
}

void render_forest_talking_game_over() {
    int frame = get_frame_index(current_state_time);
    
    // Use sync_gameover data
    if (textures.hugo_telllives.frames && textures.hugo_telllives.frame_count > 0 && 
        textures.sync_gameover && textures.sync_gameover_count > 0) {
        int sync_idx = frame;
        if (sync_idx >= textures.sync_gameover_count) sync_idx = textures.sync_gameover_count - 1;
        
        int anim_frame = textures.sync_gameover[sync_idx] - 1;  // Sync indices are 1-based
        if (anim_frame < 0) anim_frame = 0;
        if (anim_frame >= textures.hugo_telllives.frame_count) anim_frame = textures.hugo_telllives.frame_count - 1;
        
        Texture *t = textures.hugo_telllives.frames[anim_frame];
        if (t) render(t, 128, -16);
    }
}

void render_forest_win_talking() {
    int frame = get_frame_index(current_state_time);
    
    // Use sync_levelcompleted data
    if (textures.hugo_telllives.frames && textures.hugo_telllives.frame_count > 0 && 
        textures.sync_levelcompleted && textures.sync_levelcompleted_count > 0) {
        int sync_idx = frame;
        if (sync_idx >= textures.sync_levelcompleted_count) sync_idx = textures.sync_levelcompleted_count - 1;
        
        int anim_frame = textures.sync_levelcompleted[sync_idx] - 1;  // Sync indices are 1-based
        if (anim_frame < 0) anim_frame = 0;
        if (anim_frame >= textures.hugo_telllives.frame_count) anim_frame = textures.hugo_telllives.frame_count - 1;
        
        Texture *t = textures.hugo_telllives.frames[anim_frame];
        if (t) render(t, 128, -16);
    }
}

void render_forest_wait_intro() {
    int frame = get_frame_index(current_state_time);
    
    // Use sync_start data for intro talking
    if (textures.hugo_telllives.frames && textures.hugo_telllives.frame_count > 0 && 
        textures.sync_start && textures.sync_start_count > 0) {
        int sync_idx = frame;
        if (sync_idx >= textures.sync_start_count) sync_idx = textures.sync_start_count - 1;
        
        int anim_frame = textures.sync_start[sync_idx] - 1;  // Sync indices are 1-based
        if (anim_frame < 0) anim_frame = 0;
        if (anim_frame >= textures.hugo_telllives.frame_count) anim_frame = textures.hugo_telllives.frame_count - 1;
        
        Texture *t = textures.hugo_telllives.frames[anim_frame];
        if (t) render(t, 128, -16);
    }

    // Lives indicator
    for (int i = 0; i < game_ctx.lives; i++) {
        if (textures.hugo_lives) {
            render(textures.hugo_lives, 32 + i * 40, 188);
        }
    }
}

// ---------------- COLLISION ----------------

bool check_collision(int obstacle_idx) {
    ObstacleType obs = game_ctx.obstacles[obstacle_idx];
    if (obs == OBS_NONE) return false;

    // Catapult, trap, rock need jump; tree needs duck
    if (obs == OBS_CATAPULT || obs == OBS_TRAP || obs == OBS_ROCK) {
        return !game_ctx.arrow_up_focus;
    } else if (obs == OBS_TREE) {
        return !game_ctx.arrow_down_focus;
    }
    return false;
}

// ---------------- STATE PROCESSING: PLAYING / HURT / TALKING / WIN / GAME OVER ----------------

ForestState process_forest_playing(InputState state) {
    // Inputs: map to key_up / key_down from common.h
    if (!game_ctx.arrow_up_focus && !game_ctx.arrow_down_focus) {
        if (state.key_up) {
            game_ctx.arrow_up_focus = true;
            game_ctx.hugo_jumping_time = get_game_time();
        }
        if (state.key_down) {
            game_ctx.arrow_down_focus = true;
            game_ctx.hugo_crawling_time = get_game_time();
        }
    }

    // End condition
    if (game_ctx.parallax_pos > FOREST_MAX_TIME) {
        game_ctx.parallax_pos = FOREST_MAX_TIME;
        return STATE_FOREST_WIN_TALKING;
    }

    // Update parallax position
    double current_time = get_game_time();
    game_ctx.parallax_pos += current_time - game_ctx.last_time;
    game_ctx.last_time = current_time;

    // Integer / fractional position
    double int_part;
    double fract = modf(game_ctx.parallax_pos, &int_part);
    int integer = (int)int_part + 1;
    if (integer >= FOREST_MAX_TIME) {
        integer = FOREST_MAX_TIME - 1;
    }

    if (game_ctx.old_second < 0) {
        game_ctx.old_second = (int)floor(game_ctx.parallax_pos);
    }

    // Update jump / crawl timers
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

    // New second: handle collisions and sacks
    int current_second = (int)floor(game_ctx.parallax_pos);
    if (game_ctx.old_second != current_second) {

        ObstacleType obs = game_ctx.obstacles[integer];
        if (obs != OBS_NONE) {
            if (obs == OBS_CATAPULT && !game_ctx.arrow_up_focus) {
                if (audio.sfx_hugo_launch)  play(audio.sfx_hugo_launch);
                if (audio.sfx_catapult_eject) play(audio.sfx_catapult_eject);
                game_ctx.obstacles[integer] = OBS_NONE;
                return STATE_FOREST_FLYING_START;
            } else if (obs == OBS_TRAP && !game_ctx.arrow_up_focus) {
                if (audio.sfx_hugo_hittrap) play(audio.sfx_hugo_hittrap);
                game_ctx.obstacles[integer] = OBS_NONE;
                return STATE_FOREST_TRAP_ANIMATION;
            } else if (obs == OBS_ROCK && !game_ctx.arrow_up_focus) {
                if (audio.sfx_hugo_hitlog) play(audio.sfx_hugo_hitlog);
                game_ctx.obstacles[integer] = OBS_NONE;
                return STATE_FOREST_ROCK_ANIMATION;
            } else if (obs == OBS_TREE) {
                if (game_ctx.arrow_down_focus) {
                    if (audio.sfx_tree_swush) play(audio.sfx_tree_swush);
                } else {
                    if (audio.sfx_hugo_hitlog) play(audio.sfx_hugo_hitlog);
                    game_ctx.obstacles[integer] = OBS_NONE;
                    return STATE_FOREST_BRANCH_ANIMATION;
                }
            }
        }

        // Sacks when jumping
        if (game_ctx.arrow_up_focus && game_ctx.sacks[integer] != 0) {
            if (game_ctx.sacks[integer] == 1) {
                game_ctx.score += 100;
                if (audio.sfx_sack_normal) play(audio.sfx_sack_normal);
                game_ctx.sacks[integer] = 0;
            } else if (game_ctx.sacks[integer] == 2) {
                game_ctx.score += 250;
                if (audio.sfx_sack_bonus) play(audio.sfx_sack_bonus);
                game_ctx.sacks[integer] = 0;
            }
        }

        game_ctx.old_second = current_second;
    }

    // Footstep SFX every 8 frames if not jumping
    if (get_frame_index(current_state_time) % 8 == 0 && !game_ctx.arrow_up_focus) {
        int r = rand() % 5;
        Audio *walk_sfx[5] = {
            audio.sfx_hugo_walk[0],
            audio.sfx_hugo_walk[1],
            audio.sfx_hugo_walk[2],
            audio.sfx_hugo_walk[3],
            audio.sfx_hugo_walk[4]
        };
        if (walk_sfx[r]) play(walk_sfx[r]);
    }

    return STATE_FOREST_NONE;
}

// Helper: reduce lives and go to appropriate talking/game over
ForestState reduce_lives_and_transition() {
    game_ctx.lives--;
    printf("Hit obstacle! Lives remaining: %d\n", game_ctx.lives);

    if (game_ctx.lives <= 0) {
        return STATE_FOREST_TALKING_GAME_OVER;
    } else {
        return STATE_FOREST_TALKING_AFTER_HURT;
    }
}

// ---------------- HURT STATE PROCESSING ----------------

ForestState process_forest_branch_animation(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.0, 0)) {
        if (audio.sfx_birds) play(audio.sfx_birds);
    }

    if (get_frame_index_fast(current_state_time) >= textures.hugohitlog.frame_count) {
        return STATE_FOREST_BRANCH_TALKING;
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_branch_talking(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.0, 0)) {
        if (audio.speak_hitlog) play(audio.speak_hitlog);
    }

    // Use sync data to determine transition
    if (textures.sync_hitlog_count > 0 && get_frame_index(current_state_time) >= textures.sync_hitlog_count) {
        return reduce_lives_and_transition();
    }

    // Fallback
    if (get_frame_index(current_state_time) >= textures.hugohitlog_talk.frame_count) {
        return reduce_lives_and_transition();
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_flying_start(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.0, 0)) {
        if (audio.speak_catapult_up) play(audio.speak_catapult_up);
    }

    if (one_shot(current_state_time, 2.7, 1)) {
        if (audio.sfx_hugo_screenklir) play(audio.sfx_hugo_screenklir);
    }

    if (one_shot(current_state_time, 2.7, 2)) {
        if (audio.speak_catapult_hit) play(audio.speak_catapult_hit);
    }

    if (get_frame_index_fast(current_state_time) >= textures.catapult_fly.frame_count) {
        return STATE_FOREST_FLYING_TALKING;
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_flying_talking(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.0, 0)) {
        if (audio.speak_catapult_talktop) play(audio.speak_catapult_talktop);
    }

    // Use sync data to determine transition
    if (textures.sync_catapult_talktop_count > 0 && get_frame_index(current_state_time) >= textures.sync_catapult_talktop_count) {
        return STATE_FOREST_FLYING_FALLING;
    }

    // Fallback
    if (get_frame_index(current_state_time) >= textures.catapult_airtalk.frame_count) {
        return STATE_FOREST_FLYING_FALLING;
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_flying_falling(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.0, 0)) {
        if (audio.speak_catapult_down) play(audio.speak_catapult_down);
        if (audio.sfx_hugo_crash)       play(audio.sfx_hugo_crash);
    }

    if (get_frame_index_fast(current_state_time) >= textures.catapult_fall.frame_count) {
        return STATE_FOREST_FLYING_FALLING_HANG_ANIMATION;
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_flying_falling_hang_animation(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.0, 0)) {
        if (audio.sfx_hugo_hangstart) play(audio.sfx_hugo_hangstart);
    }

    if (get_frame_index_fast(current_state_time) >= textures.catapult_hang.frame_count) {
        return STATE_FOREST_FLYING_FALLING_HANG_TALKING;
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_flying_falling_hang_talking(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.0, 0)) {
        if (audio.speak_catapult_hang) play(audio.speak_catapult_hang);
        if (audio.sfx_hugo_hang)       play(audio.sfx_hugo_hang);
    }

    // Use sync data to determine transition
    if (textures.sync_catapult_hang_count > 0 && get_frame_index(current_state_time) >= textures.sync_catapult_hang_count) {
        return reduce_lives_and_transition();
    }

    // Fallback: time-based
    if (get_state_time(current_state_time) > 3.0) {
        return reduce_lives_and_transition();
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_rock_animation(InputState state) {
    (void)state;
    if (get_frame_index_fast(current_state_time) >= textures.hugo_lookrock.frame_count) {
        return STATE_FOREST_ROCK_HIT_ANIMATION;
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_rock_hit_animation(InputState state) {
    (void)state;
    if (get_frame_index_fast(current_state_time) >= textures.hit_rock.frame_count) {
        return STATE_FOREST_ROCK_TALKING;
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_rock_talking(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.0, 0)) {
        if (audio.speak_rock) play(audio.speak_rock);
    }

    // Use sync data if available
    if (textures.sync_rock_count > 0 && get_frame_index(current_state_time) >= textures.sync_rock_count) {
        return reduce_lives_and_transition();
    }

    // Fallback: time-based
    if (get_state_time(current_state_time) > 3.0) {
        return reduce_lives_and_transition();
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_trap_animation(InputState state) {
    (void)state;
    if (get_frame_index(current_state_time) >= textures.hugo_traphurt.frame_count) {
        return STATE_FOREST_TRAP_TALKING;
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_trap_talking(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.0, 0)) {
        if (audio.speak_trap) play(audio.speak_trap);
    }

    // Use sync data if available
    if (textures.sync_trap_count > 0 && get_frame_index(current_state_time) >= textures.sync_trap_count) {
        return reduce_lives_and_transition();
    }

    // Fallback: time-based
    if (get_state_time(current_state_time) > 3.0) {
        return reduce_lives_and_transition();
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_scylla_button(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.5, 0)) {
        if (audio.sfx_lightning_warning) play(audio.sfx_lightning_warning);
    }

    if (get_state_time(current_state_time) > 2.0) {
        return STATE_FOREST_PLAYING;
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_talking_after_hurt(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.5, 0)) {
        if (audio.sfx_hugo_knock) play(audio.sfx_hugo_knock);
    }

    if (one_shot(current_state_time, 0.0, 1)) {
        if (audio.sfx_hugo_knock) play(audio.sfx_hugo_knock);
        if (game_ctx.lives == 1) {
            if (audio.speak_lastlife) play(audio.speak_lastlife);
        } else {
            if (audio.speak_dieonce)  play(audio.speak_dieonce);
        }
    }

    // Use sync data to determine when to transition
    int sync_count = (game_ctx.lives == 1) ? textures.sync_lastlife_count : textures.sync_dieonce_count;
    if (sync_count > 0 && get_frame_index(current_state_time) >= sync_count) {
        return STATE_FOREST_PLAYING;
    }

    // Fallback: after some time, continue
    if (get_state_time(current_state_time) > 4.0) {
        return STATE_FOREST_PLAYING;
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_talking_game_over(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.0, 0)) {
        if (audio.speak_gameover) play(audio.speak_gameover);
    }

    // Use sync data to determine when to transition
    if (textures.sync_gameover_count > 0 && get_frame_index(current_state_time) >= textures.sync_gameover_count) {
        return STATE_FOREST_END;
    }

    // Fallback: after some time, end
    if (get_state_time(current_state_time) > 5.0) {
        return STATE_FOREST_END;
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_wait_intro(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.0, 0)) {
        if (audio.speak_start) play(audio.speak_start);
    }

    // Use sync data to determine when to transition to playing
    if (textures.sync_start_count > 0 && get_frame_index(current_state_time) >= textures.sync_start_count) {
        return STATE_FOREST_PLAYING;
    }

    // Fallback timeout
    if (get_state_time(current_state_time) > 3.0) {
        return STATE_FOREST_PLAYING;
    }
    return STATE_FOREST_NONE;
}

ForestState process_forest_win_talking(InputState state) {
    (void)state;
    if (one_shot(current_state_time, 0.0, 0)) {
        if (audio.speak_levelcompleted) play(audio.speak_levelcompleted);
    }

    // Use sync data to determine when to transition
    if (textures.sync_levelcompleted_count > 0 && get_frame_index(current_state_time) >= textures.sync_levelcompleted_count) {
        return STATE_FOREST_END;
    }

    // Fallback timeout
    if (get_state_time(current_state_time) > 5.0) {
        return STATE_FOREST_END;
    }
    return STATE_FOREST_NONE;
}

// Legacy win / game over (kept for compatibility)

ForestState process_win(InputState state) {
    (void)state;
    if (get_state_time(current_state_time) > 3.0) {
        return STATE_FOREST_END;
    }
    return STATE_FOREST_NONE;
}

ForestState process_game_over(InputState state) {
    (void)state;
    if (get_state_time(current_state_time) > 5.0) {
        return STATE_FOREST_END;
    }
    return STATE_FOREST_NONE;
}


GameState process_forest(InputState state){

    ForestState next_state = STATE_FOREST_NONE;

    switch (current_forest_state) {
    case STATE_FOREST_WAIT_INTRO:
        next_state = process_forest_wait_intro(state);
        break;
    case STATE_FOREST_PLAYING:
        next_state = process_forest_playing(state);
        break;
    case STATE_FOREST_BRANCH_ANIMATION:
        next_state = process_forest_branch_animation(state);
        break;
    case STATE_FOREST_BRANCH_TALKING:
        next_state = process_forest_branch_talking(state);
        break;
    case STATE_FOREST_FLYING_START:
        next_state = process_forest_flying_start(state);
        break;
    case STATE_FOREST_FLYING_TALKING:
        next_state = process_forest_flying_talking(state);
        break;
    case STATE_FOREST_FLYING_FALLING:
        next_state = process_forest_flying_falling(state);
        break;
    case STATE_FOREST_FLYING_FALLING_HANG_ANIMATION:
        next_state = process_forest_flying_falling_hang_animation(state);
        break;
    case STATE_FOREST_FLYING_FALLING_HANG_TALKING:
        next_state = process_forest_flying_falling_hang_talking(state);
        break;
    case STATE_FOREST_ROCK_ANIMATION:
        next_state = process_forest_rock_animation(state);
        break;
    case STATE_FOREST_ROCK_HIT_ANIMATION:
        next_state = process_forest_rock_hit_animation(state);
        break;
    case STATE_FOREST_ROCK_TALKING:
        next_state = process_forest_rock_talking(state);
        break;
    case STATE_FOREST_TRAP_ANIMATION:
        next_state = process_forest_trap_animation(state);
        break;
    case STATE_FOREST_TRAP_TALKING:
        next_state = process_forest_trap_talking(state);
        break;
    case STATE_FOREST_SCYLLA_BUTTON:
        next_state = process_forest_scylla_button(state);
        break;
    case STATE_FOREST_TALKING_AFTER_HURT:
        next_state = process_forest_talking_after_hurt(state);
        break;
    case STATE_FOREST_TALKING_GAME_OVER:
        next_state = process_forest_talking_game_over(state);
        break;
    case STATE_FOREST_WIN_TALKING:
        next_state = process_forest_win_talking(state);
        break;
    default:
        break;
    }

     // Handle state transitions
    if (next_state != STATE_FOREST_NONE) {
        if (next_state == STATE_FOREST_END) {
            return STATE_CAVE;
        }
        
        current_forest_state = next_state;
        reset_state_events();
        current_state_time = get_game_time();
    }
       
    return STATE_NONE;
}

void render_forest(){
    switch (current_forest_state) {
    case STATE_FOREST_WAIT_INTRO:
        render_forest_wait_intro();
        break;
    case STATE_FOREST_PLAYING:
        render_forest_playing();
        break;
    case STATE_FOREST_BRANCH_ANIMATION:
        render_forest_branch_animation();
        break;
    case STATE_FOREST_BRANCH_TALKING:
        render_forest_branch_talking();
        break;
    case STATE_FOREST_FLYING_START:
        render_forest_flying_start();
        break;
    case STATE_FOREST_FLYING_TALKING:
        render_forest_flying_talking();
        break;
    case STATE_FOREST_FLYING_FALLING:
        render_forest_flying_falling();
        break;
    case STATE_FOREST_FLYING_FALLING_HANG_ANIMATION:
        render_forest_flying_falling_hang_animation();
        break;
    case STATE_FOREST_FLYING_FALLING_HANG_TALKING:
        render_forest_flying_falling_hang_talking();
        break;
    case STATE_FOREST_ROCK_ANIMATION:
        render_forest_rock_animation();
        break;
    case STATE_FOREST_ROCK_HIT_ANIMATION:
        render_forest_rock_hit_animation();
        break;
    case STATE_FOREST_ROCK_TALKING:
        render_forest_rock_talking();
        break;
    case STATE_FOREST_TRAP_ANIMATION:
        render_forest_trap_animation();
        break;
    case STATE_FOREST_TRAP_TALKING:
        render_forest_trap_talking();
        break;
    case STATE_FOREST_SCYLLA_BUTTON:
        render_forest_scylla_button();
        break;
    case STATE_FOREST_TALKING_AFTER_HURT:
        render_forest_talking_after_hurt();
        break;
    case STATE_FOREST_TALKING_GAME_OVER:
        render_forest_talking_game_over();
        break;
    case STATE_FOREST_WIN_TALKING:
        render_forest_win_talking();
        break;
    default:
        break;
    }
}

void on_enter_forest(){
    on_enter_forest_playing();
}
