/*
 * Hugo Lite - Single Player lite Version, easily portable to other platforms
 * A simplified version of the Hugo forest and cave mini games
 * Based on the Python implementation in ../game/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "common.h"
#include "config.h"
#include "state.h"
#include "forest.h"
#include "cave.h"
#include "cJSON.h"

#define INSTRUCTIONS_TIMEOUT 3

GameContext game_ctx;
GameTextures textures;
GameAudio audio;

GameState process_instructions(InputState state) {
    if (state.key_start || get_state_time() > INSTRUCTIONS_TIMEOUT) {
        return STATE_FOREST_WAIT_INTRO;
    }
    return STATE_NONE;
}

void render_instructions() {
    render(textures.instruction_screen, 0, 0);
}

ProcessFunc process_funcs[STATE_END + 1] = {
    NULL,
    process_instructions,
    process_forest_branch_animation,
    process_forest_branch_talking,
    process_forest_flying_falling,
    process_forest_flying_falling_hang_animation,
    process_forest_flying_falling_hang_talking,
    process_forest_flying_start,
    process_forest_flying_talking,
    process_forest_rock_animation,
    process_forest_rock_hit_animation,
    process_forest_rock_talking,
    process_forest_trap_animation,
    process_forest_trap_talking,
    process_forest_playing,
    process_forest_scylla_button,
    process_forest_talking_after_hurt,
    process_forest_talking_game_over,
    process_forest_wait_intro,
    process_forest_win_talking,
    process_cave_climbing,
    process_cave_family_cage_opens,
    process_cave_family_happy,
    process_cave_going_rope,
    process_cave_lost,
    process_cave_lost_spring,
    process_cave_scylla_lost,
    process_cave_scylla_spring,
    process_cave_talking_before_climb,
    process_cave_waiting_before_talking,
    process_cave_waiting_input,
};

RenderFunc render_funcs[STATE_END + 1] = {
    NULL,
    render_instructions,
    render_forest_branch_animation,
    render_forest_branch_talking,
    render_forest_flying_falling,
    render_forest_flying_falling_hang_animation,
    render_forest_flying_falling_hang_talking,
    render_forest_flying_start,
    render_forest_flying_talking,
    render_forest_rock_animation,
    render_forest_rock_hit_animation,
    render_forest_rock_talking,
    render_forest_trap_animation,
    render_forest_trap_talking,
    render_forest_playing,
    render_forest_scylla_button,
    render_forest_talking_after_hurt,
    render_forest_talking_game_over,
    render_forest_wait_intro,
    render_forest_win_talking,
    render_cave_climbing,
    render_cave_family_cage_opens,
    render_cave_family_happy,
    render_cave_going_rope,
    render_cave_lost,
    render_cave_lost_spring,
    render_cave_scylla_lost,
    render_cave_scylla_spring,
    render_cave_talking_before_climb,
    render_cave_waiting_before_talking,
    render_cave_waiting_input
};

RenderFunc on_enter_funcs[STATE_END + 1] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    on_enter_forest_playing,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    on_enter_cave_climbing,
    on_enter_cave_family_cage_opens,
    on_enter_cave_family_happy,
    on_enter_cave_going_rope,
    on_enter_cave_lost,
    on_enter_cave_lost_spring,
    on_enter_cave_scylla_lost,
    on_enter_cave_scylla_spring,
    on_enter_cave_talking_before_climb,
    on_enter_cave_waiting_before_talking,
    on_enter_cave_waiting_input
};


const char * game_state_name[] = {
    "STATE_NONE",
    "STATE_INSTRUCTIONS",

    // Forest states
    "STATE_FOREST_BRANCH_ANIMATION",
    "STATE_FOREST_BRANCH_TALKING",
    "STATE_FOREST_FLYING_FALLING",
    "STATE_FOREST_FLYING_FALLING_HANG_ANIMATION",
    "STATE_FOREST_FLYING_FALLING_HANG_TALKING",
    "STATE_FOREST_FLYING_START",
    "STATE_FOREST_FLYING_TALKING",
    "STATE_FOREST_ROCK_ANIMATION",
    "STATE_FOREST_ROCK_HIT_ANIMATION",
    "STATE_FOREST_ROCK_TALKING",
    "STATE_FOREST_TRAP_ANIMATION",
    "STATE_FOREST_TRAP_TALKING",
    "STATE_FOREST_PLAYING",
    "STATE_FOREST_SCYLLA_BUTTON",
    "STATE_FOREST_TALKING_AFTER_HURT",
    "STATE_FOREST_TALKING_GAME_OVER",
    "STATE_FOREST_WAIT_INTRO",
    "STATE_FOREST_WIN_TALKING",

    // Cave bonus game states
    "STATE_CAVE_CLIMBING",
    "STATE_CAVE_FAMILY_CAGE_OPENS",
    "STATE_CAVE_FAMILY_HAPPY",
    "STATE_CAVE_GOING_ROPE",
    "STATE_CAVE_LOST",
    "STATE_CAVE_LOST_SPRING",
    "STATE_CAVE_SCYLLA_LOST",
    "STATE_CAVE_SCYLLA_SPRING",
    "STATE_CAVE_TALKING_BEFORE_CLIMB",
    "STATE_CAVE_WAITING_BEFORE_TALKING",
    "STATE_CAVE_WAITING_INPUT",

    // End state
    "STATE_END"
};

Texture* animation_get_sync_frame(Animation animation, int frame_index) {
    if(animation.sync_data == NULL) {
        printf("Animation has no sync data!\n");
        return NULL;
    }
    
    if (frame_index >= animation.sync_count) {
        frame_index = animation.sync_count - 1;
    }
    
    int frame = animation.sync_data[frame_index] - 1;
    if (frame < 0) {
        printf("Animation index less than 0!\n");
        frame = 0;
    }
    
    if (frame >= animation.frame_count) {
        printf("Animation index out of range!\n");
        frame = animation.frame_count - 1;
    }
    
    return animation.frames[frame];
}
// Get a regular animation frame
Texture* animation_get_frame(Animation animation, int frame_index) {
    if (frame_index >= animation.frame_count) {
        printf("Animation index out of range!\n");
        frame_index = animation.frame_count - 1;
    }
    
    return animation.frames[frame_index];
}

int load_animation_frames(Texture** textures, const char* data_dir, const char* rel_path, int start, int end) {
    int loaded = 0;
    for (int i = start; i <= end; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s%s_%d.png", data_dir, rel_path, i);
        textures[i - start] = load_texture(path);
        if (textures[i - start]) {
            loaded++;
        }
    }
    if (loaded < (end - start + 1)) {
        printf("Warning: Some animation frames failed to load for %s%s\n", data_dir, rel_path);
    }
    return loaded;
}

Animation load_animation_sequence(const char* data_dir, const char* rel_path, int start, int end, const char* sync_file) {
    Animation anim;
    anim.frame_count = end - start + 1;
    anim.frames = (Texture**)malloc(anim.frame_count * sizeof(Texture*));
    if (!anim.frames) {
        anim.frame_count = 0;
        anim.sync_data = NULL;
        anim.sync_count = 0;
        return anim;
    }
    
    // Load frames
    int loaded = 0;
    for (int i = start; i <= end; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s%s_%d.png", data_dir, rel_path, i);
        anim.frames[i - start] = load_texture(path);
        if (anim.frames[i - start]) {
            loaded++;
        }
    }
    
    // Load sync data if provided
    anim.sync_data = NULL;
    anim.sync_count = 0;
    
    if (sync_file != NULL) {
        char sync_path[512];
        snprintf(sync_path, sizeof(sync_path), "%s/%s", data_dir, sync_file);
        
        FILE* fp = fopen(sync_path, "rb");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            long length = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            
            char* json_str = (char*)malloc(length + 1);
            if (json_str) {
                fread(json_str, 1, length, fp);
                json_str[length] = '\0';
                
                cJSON* root = cJSON_Parse(json_str);
                if (root) {
                    cJSON* indices = cJSON_GetObjectItem(root, "frameIndices");
                    if (cJSON_IsArray(indices)) {
                        anim.sync_count = cJSON_GetArraySize(indices);
                        anim.sync_data = (int*)malloc(anim.sync_count * sizeof(int));
                        
                        if (anim.sync_data) {
                            for (int i = 0; i < anim.sync_count; i++) {
                                cJSON* item = cJSON_GetArrayItem(indices, i);
                                anim.sync_data[i] = cJSON_GetNumberValue(item);
                            }
                        }
                    }
                    cJSON_Delete(root);
                }
                free(json_str);
            }
            fclose(fp);
        } else {
            printf("Warning: Could not load sync file %s\n", sync_path);
        }
    }
    
    return anim;
}

// Helper to load standalone sync data (for talking animations)
int* load_sync_data(const char* data_dir, const char* sync_file, int* out_count) {
    char sync_path[512];
    snprintf(sync_path, sizeof(sync_path), "%s/ForestData/Syncs/%s.json", data_dir, sync_file);
    
    int* sync_data = NULL;
    *out_count = 0;
    
    FILE* fp = fopen(sync_path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        char* json_str = (char*)malloc(length + 1);
        if (json_str) {
            fread(json_str, 1, length, fp);
            json_str[length] = '\0';
            
            cJSON* root = cJSON_Parse(json_str);
            if (root) {
                cJSON* indices = cJSON_GetObjectItem(root, "frameIndices");
                if (cJSON_IsArray(indices)) {
                    *out_count = cJSON_GetArraySize(indices);
                    sync_data = (int*)malloc(*out_count * sizeof(int));
                    
                    if (sync_data) {
                        for (int i = 0; i < *out_count; i++) {
                            cJSON* item = cJSON_GetArrayItem(indices, i);
                            sync_data[i] = cJSON_GetNumberValue(item);
                        }
                    }
                }
                cJSON_Delete(root);
            }
            free(json_str);
        }
        fclose(fp);
    } else {
        printf("Warning: Could not load sync file %s\n", sync_path);
    }
    
    return sync_data;
}

void init_textures(const char *data_dir) {
    // Initialize all to NULL
    memset(&textures, 0, sizeof(GameTextures));
    
    char path[512];
    
    // Load fixed assets (these should always be available)
    textures.instruction_screen = load_texture("../game/resources/images/instruction_Forest.png");
    textures.bg_gradient = load_texture("../game/resources/fixed_assets/gradient.bmp");
    
    // Load arrow buttons
    for (int i = 0; i < 4; i++) {
        snprintf(path, sizeof(path), "../game/resources/fixed_assets/arrows.cgf_%d.png", i);
        textures.arrows[i] = load_texture(path);
    }
    
    // Load background layers from ForestData
    snprintf(path, sizeof(path), "%s/ForestData/gfx/hillsday.cgf_0.png", data_dir);
    textures.bg_hillsday = load_texture(path);
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/paratrees.cgf_0.png", data_dir);
    textures.bg_trees = load_texture(path);
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/paraground.cgf_0.png", data_dir);
    textures.bg_ground = load_texture(path);
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/GRASS.cgf_0.png", data_dir);
    textures.grass = load_texture(path);
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/LEAVES1.cgf_0.png", data_dir);
    textures.leaves1 = load_texture(path);
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/LEAVES2.cgf_0.png", data_dir);
    textures.leaves2 = load_texture(path);
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/SCOREBRD.bmp", data_dir);
    textures.scoreboard = load_texture(path);
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/WALL.cgf_0.png", data_dir);
    textures.end_mountain = load_texture(path);
    
    // Load Hugo animations
    load_animation_frames(textures.hugo_side, data_dir, "/ForestData/gfx/hugoside.cgf", 0, 7);
    load_animation_frames(textures.hugo_jump, data_dir, "/ForestData/gfx/hugohop.cgf", 0, 2);
    load_animation_frames(textures.hugo_crawl, data_dir, "/ForestData/gfx/kravle.cgf", 0, 7);
    // Hugo talking animations need sync data for mouth movements
    textures.hugo_telllives = load_animation_sequence(data_dir, "/ForestData/gfx/hugo_hello.cgf", 0, 15, NULL);
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/hand1.cgf_0.png", data_dir);
    textures.hugo_hand1 = load_texture(path);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/hand2.cgf_0.png", data_dir);
    textures.hugo_hand2 = load_texture(path);

    // Load obstacles
    load_animation_frames(textures.catapult, data_dir, "/ForestData/gfx/catapult.cgf", 0, 7);
    load_animation_frames(textures.trap, data_dir, "/ForestData/gfx/faelde.cgf", 0, 5);
    load_animation_frames(textures.rock, data_dir, "/ForestData/gfx/stone.cgf", 0, 7);
    load_animation_frames(textures.tree, data_dir, "/ForestData/gfx/branch-swing.cgf", 0, 6);
    load_animation_frames(textures.sack, data_dir, "/ForestData/gfx/saek.cgf", 0, 3);
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/lonetree.cgf_0.png", data_dir);
    textures.lone_tree = load_texture(path);
    
    // Load HUD elements
    snprintf(path, sizeof(path), "%s/ForestData/gfx/SCORES.cgf_0.png", data_dir);
    textures.score_numbers = load_texture(path);
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/HUGOSTAT.cgf_0.png", data_dir);
    textures.hugo_lives = load_texture(path);
    
    // Load forest hurt animation sequences
    textures.hugohitlog = load_animation_sequence(data_dir, "/ForestData/gfx/BRANCH-GROGGY.til", 0, 42, NULL);
    textures.hugohitlog_talk = load_animation_sequence(data_dir, "/ForestData/gfx/BRANCH-SPEAK.til", 0, 17, NULL);
    textures.catapult_fly = load_animation_sequence(data_dir, "/ForestData/gfx/HGKATFLY.til", 0, 113, NULL);
    textures.catapult_fall = load_animation_sequence(data_dir, "/ForestData/gfx/HGKATFLY.til", 115, 189, NULL);
    textures.catapult_airtalk = load_animation_sequence(data_dir, "/ForestData/gfx/CATAPULT-SPEAK.til", 0, 15, NULL);
    textures.catapult_hang = load_animation_sequence(data_dir, "/ForestData/gfx/HGKATHNG.TIL", 0, 12, NULL);
    textures.catapult_hangspeak = load_animation_sequence(data_dir, "/ForestData/gfx/hanging_mouth.cgf", 0, 11, NULL);
    textures.hugo_lookrock = load_animation_sequence(data_dir, "/ForestData/gfx/hugo-rock.til", 0, 14, NULL);
    textures.hit_rock = load_animation_sequence(data_dir, "/ForestData/gfx/HGROCK.TIL", 0, 60, NULL);
    textures.hit_rock_sync = load_animation_sequence(data_dir, "/ForestData/gfx/MSYNCRCK.TIL", 0, 17, NULL);
    textures.hugo_traphurt = load_animation_sequence(data_dir, "/ForestData/gfx/TRAP-HURTS.til", 0, 9, NULL);
    textures.hugo_traptalk = load_animation_sequence(data_dir, "/ForestData/gfx/traptalk.til", 0, 15, NULL);
    
    // Load sync data for talking animations
    textures.sync_start = load_sync_data(data_dir, "005-01.oos", &textures.sync_start_count);
    textures.sync_rock = load_sync_data(data_dir, "005-02.oos", &textures.sync_rock_count);
    textures.sync_dieonce = load_sync_data(data_dir, "005-03.oos", &textures.sync_dieonce_count);
    textures.sync_trap = load_sync_data(data_dir, "005-04.oos", &textures.sync_trap_count);
    textures.sync_lastlife = load_sync_data(data_dir, "005-05.oos", &textures.sync_lastlife_count);
    textures.sync_catapult_talktop = load_sync_data(data_dir, "005-08.oos", &textures.sync_catapult_talktop_count);
    textures.sync_catapult_hang = load_sync_data(data_dir, "005-10.oos", &textures.sync_catapult_hang_count);
    textures.sync_hitlog = load_sync_data(data_dir, "005-11.oos", &textures.sync_hitlog_count);
    textures.sync_gameover = load_sync_data(data_dir, "005-12.oos", &textures.sync_gameover_count);
    textures.sync_levelcompleted = load_sync_data(data_dir, "005-13.oos", &textures.sync_levelcompleted_count);
    
    // Load cave animation sequences
    textures.cave_talks = load_animation_sequence(data_dir, "/RopeOutroData/gfx/STAIRS.TIL", 0, 12, "/RopeOutroData/Syncs/002-06.oos.json");
    textures.cave_climbs = load_animation_sequence(data_dir, "/RopeOutroData/gfx/STAIRS.TIL", 11, 51, NULL);
    textures.cave_first_rope = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 0, 32, NULL);
    textures.cave_second_rope = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 33, 72, NULL);
    textures.cave_third_rope = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 73, 121, NULL);
    textures.cave_scylla_leaves = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 122, 177, NULL);
    textures.cave_scylla_bird = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 178, 240, NULL);
    textures.cave_scylla_ropes = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 241, 283, NULL);
    textures.cave_scylla_spring = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 284, 318, NULL);
    textures.cave_family_cage = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 319, 352, NULL);
    textures.cave_hugo_puff_first = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASEDIE.TIL", 122, 166, NULL);
    textures.cave_hugo_puff_second = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASEDIE.TIL", 167, 211, NULL);
    textures.cave_hugo_puff_third = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASEDIE.TIL", 212, 256, NULL);
    textures.cave_hugo_spring = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASEDIE.TIL", 257, 295, NULL);
    textures.cave_happy = load_animation_sequence(data_dir, "/RopeOutroData/gfx/HAPPY.TIL", 0, 111, NULL);
    
    // Load cave hugo sprite
    snprintf(path, sizeof(path), "%s/RopeOutroData/gfx/hugo.cgf_0.png", data_dir);
    textures.cave_hugo_sprite = load_texture(path);
}

Mix_Chunk* load_audio(const char* path) {
    Mix_Chunk* chunk = Mix_LoadWAV(path);
    if (!chunk) {
        printf("Warning: Could not load audio %s: %s\n", path, Mix_GetError());
        return STATE_NONE;
    }
    return chunk;
}

void init_audio(const char *data_dir) {
    // Initialize all to NULL
    memset(&audio, 0, sizeof(GameAudio));
    
    char path[512];
    int loaded_count = 0;
    
    // Load forest speech
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-01.wav", data_dir);
    audio.speak_start = load_audio(path);
    if (audio.speak_start) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-02.wav", data_dir);
    audio.speak_rock = load_audio(path);
    if (audio.speak_rock) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-03.wav", data_dir);
    audio.speak_dieonce = load_audio(path);
    if (audio.speak_dieonce) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-04.wav", data_dir);
    audio.speak_trap = load_audio(path);
    if (audio.speak_trap) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-05.wav", data_dir);
    audio.speak_lastlife = load_audio(path);
    if (audio.speak_lastlife) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-06.wav", data_dir);
    audio.speak_catapult_up = load_audio(path);
    if (audio.speak_catapult_up) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-07.wav", data_dir);
    audio.speak_catapult_hit = load_audio(path);
    if (audio.speak_catapult_hit) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-08.wav", data_dir);
    audio.speak_catapult_talktop = load_audio(path);
    if (audio.speak_catapult_talktop) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-09.wav", data_dir);
    audio.speak_catapult_down = load_audio(path);
    if (audio.speak_catapult_down) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-10.wav", data_dir);
    audio.speak_catapult_hang = load_audio(path);
    if (audio.speak_catapult_hang) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-11.wav", data_dir);
    audio.speak_hitlog = load_audio(path);
    if (audio.speak_hitlog) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-12.wav", data_dir);
    audio.speak_gameover = load_audio(path);
    if (audio.speak_gameover) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/speaks/005-13.wav", data_dir);
    audio.speak_levelcompleted = load_audio(path);
    if (audio.speak_levelcompleted) loaded_count++;
    
    // Load forest sound effects
    snprintf(path, sizeof(path), "%s/ForestData/sfx/atmos-lp.wav", data_dir);
    audio.sfx_bg_atmosphere = load_audio(path);
    if (audio.sfx_bg_atmosphere) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/warning.wav", data_dir);
    audio.sfx_lightning_warning = load_audio(path);
    if (audio.sfx_lightning_warning) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/knock.wav", data_dir);
    audio.sfx_hugo_knock = load_audio(path);
    if (audio.sfx_hugo_knock) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/crunch.wav", data_dir);
    audio.sfx_hugo_hittrap = load_audio(path);
    if (audio.sfx_hugo_hittrap) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/skriid.wav", data_dir);
    audio.sfx_hugo_launch = load_audio(path);
    if (audio.sfx_hugo_launch) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/sack-norm.wav", data_dir);
    audio.sfx_sack_normal = load_audio(path);
    if (audio.sfx_sack_normal) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/sack.wav", data_dir);
    audio.sfx_sack_bonus = load_audio(path);
    if (audio.sfx_sack_bonus) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/wush.wav", data_dir);
    audio.sfx_tree_swush = load_audio(path);
    if (audio.sfx_tree_swush) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/bell.wav", data_dir);
    audio.sfx_hugo_hitlog = load_audio(path);
    if (audio.sfx_hugo_hitlog) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/fjeder.wav", data_dir);
    audio.sfx_catapult_eject = load_audio(path);
    if (audio.sfx_catapult_eject) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/birds-lp.wav", data_dir);
    audio.sfx_birds = load_audio(path);
    if (audio.sfx_birds) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/hit_screen.wav", data_dir);
    audio.sfx_hugo_hitscreen = load_audio(path);
    if (audio.sfx_hugo_hitscreen) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/klirr.wav", data_dir);
    audio.sfx_hugo_screenklir = load_audio(path);
    if (audio.sfx_hugo_screenklir) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/kineser.wav", data_dir);
    audio.sfx_hugo_crash = load_audio(path);
    if (audio.sfx_hugo_crash) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/knage-start.wav", data_dir);
    audio.sfx_hugo_hangstart = load_audio(path);
    if (audio.sfx_hugo_hangstart) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/sfx/knage-lp.wav", data_dir);
    audio.sfx_hugo_hang = load_audio(path);
    if (audio.sfx_hugo_hang) loaded_count++;
    
    // Load walk sounds
    for (int i = 0; i < 5; i++) {
        snprintf(path, sizeof(path), "%s/ForestData/sfx/fumle%d.wav", data_dir, i);
        audio.sfx_hugo_walk[i] = load_audio(path);
        if (audio.sfx_hugo_walk[i]) loaded_count++;
    }
    
    // Load cave speech
    snprintf(path, sizeof(path), "%s/RopeOutroData/speak/002-05.wav", data_dir);
    audio.cave_her_er_vi = load_audio(path);
    if (audio.cave_her_er_vi) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/speak/002-06.wav", data_dir);
    audio.cave_trappe_snak = load_audio(path);
    if (audio.cave_trappe_snak) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/speak/002-07.wav", data_dir);
    audio.cave_nu_kommer_jeg = load_audio(path);
    if (audio.cave_nu_kommer_jeg) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/speak/002-08.wav", data_dir);
    audio.cave_afskylia_snak = load_audio(path);
    if (audio.cave_afskylia_snak) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/speak/002-09.wav", data_dir);
    audio.cave_hugo_katapult = load_audio(path);
    if (audio.cave_hugo_katapult) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/speak/002-10.wav", data_dir);
    audio.cave_hugo_skyd_ud = load_audio(path);
    if (audio.cave_hugo_skyd_ud) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/speak/002-11.wav", data_dir);
    audio.cave_afskylia_skyd_ud = load_audio(path);
    if (audio.cave_afskylia_skyd_ud) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/speak/002-12.wav", data_dir);
    audio.cave_hugoline_tak = load_audio(path);
    if (audio.cave_hugoline_tak) loaded_count++;
    
    // Load cave sound effects
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/BA-13.WAV", data_dir);
    audio.cave_stemning = load_audio(path);
    if (audio.cave_stemning) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/BA-15.WAV", data_dir);
    audio.cave_fodtrin1 = load_audio(path);
    if (audio.cave_fodtrin1) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/BA-16.WAV", data_dir);
    audio.cave_fodtrin2 = load_audio(path);
    if (audio.cave_fodtrin2) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/BA-17.WAV", data_dir);
    audio.cave_hiv_i_reb = load_audio(path);
    if (audio.cave_hiv_i_reb) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/BA-18.WAV", data_dir);
    audio.cave_fjeder = load_audio(path);
    if (audio.cave_fjeder) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/BA-21.WAV", data_dir);
    audio.cave_pre_puf = load_audio(path);
    if (audio.cave_pre_puf) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/BA-22.WAV", data_dir);
    audio.cave_puf = load_audio(path);
    if (audio.cave_puf) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/BA-24.WAV", data_dir);
    audio.cave_tast_trykket = load_audio(path);
    if (audio.cave_tast_trykket) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/BA-101.WAV", data_dir);
    audio.cave_pre_fanfare = load_audio(path);
    if (audio.cave_pre_fanfare) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/BA-102.WAV", data_dir);
    audio.cave_fanfare = load_audio(path);
    if (audio.cave_fanfare) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/BA-104.WAV", data_dir);
    audio.cave_fugle_skrig = load_audio(path);
    if (audio.cave_fugle_skrig) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/HEXHAHA.WAV", data_dir);
    audio.cave_trappe_grin = load_audio(path);
    if (audio.cave_trappe_grin) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/SKRIG.WAV", data_dir);
    audio.cave_skrig = load_audio(path);
    if (audio.cave_skrig) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/RopeOutroData/SFX/COUNTER.WAV", data_dir);
    audio.cave_score_counter = load_audio(path);
    if (audio.cave_score_counter) loaded_count++;
    
    printf("Loaded %d audio files\n", loaded_count);
}


// Main game loop
void game_loop() {
    bool quit = false;
        
    // Initialize
    init_game_context();
    state_info.current_state = STATE_INSTRUCTIONS;
    state_info.state_start_time = get_game_time();
    srand(time(NULL));
    InputState input_state = {0};
    
    while (!quit && state_info.current_state != STATE_END) {
        // Handle events

        if(render_getevents(&input_state)){
            quit = true;
            break;
        }

        GameState new_state = process_funcs[state_info.current_state](input_state);
        
        if(new_state != STATE_NONE) {
            printf("State transition: %s -> %s\n", game_state_name[state_info.current_state], game_state_name[new_state]);
            state_info.current_state = new_state;
            state_info.state_start_time = get_game_time();
            reset_state_events();
            OnEnterFunc on_enter_func = on_enter_funcs[state_info.current_state];
            if (on_enter_func != NULL) {
                on_enter_func();
            }
        }

        render_funcs[state_info.current_state]();
        render_step();
    }
    
    printf("Game ended. Final score: %d\n", game_ctx.score);
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <decompressed_data_directory>\n", argv[0]);
        return 1;
    }
    
    printf("Hugo Lite - Single Player Forest Game\n");
    printf("Based on the Hugo TV game from the 90s\n");
    printf("Controls: Press 2/UP to JUMP, 8/DOWN to DUCK, 5 to START, ESC to quit\n\n");
    
    const char *data_dir = argv[1];
    if (!render_init(data_dir)) {
        printf("Failed to initialize renderer!\n");
        return 1;
    }
    init_textures(data_dir);
    init_audio(data_dir);
    
    game_loop();
    render_cleanup();
    
    return 0;
}
