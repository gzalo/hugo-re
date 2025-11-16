/*
 * Hugo Lite - Single Player C/SDL2 Version
 * A simplified version of the Hugo forest and cave mini games
 * Based on the Python implementation in ../game/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "common.h"
#include "state.h"
#include "forest.h"
#include "cave.h"

GameState process_instructions(InputState state) {
    if (state.key_start || get_state_time() > INSTRUCTIONS_TIMEOUT) {
        return STATE_FOREST_WAIT_INTRO;
    }
    return STATE_NONE;
}

void render_instructions() {
    SDL_RenderCopy(renderer, textures.instruction_screen, NULL, NULL);
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

SDL_Texture* animation_get_sync_frame(Animation animation, int frame_index) {
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
SDL_Texture* animation_get_frame(Animation animation, int frame_index) {
    if (frame_index >= animation.frame_count) {
        printf("Animation index out of range!\n");
        frame_index = animation.frame_count - 1;
    }
    
    return animation.frames[frame_index];
}

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameContext game_ctx;
GameTextures textures;
GameAudio audio;


SDL_Texture* load_texture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Warning: Could not load image %s: %s\n", path, IMG_GetError());
        return STATE_NONE;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        printf("Warning: Could not create texture from %s: %s\n", path, SDL_GetError());
    }
    
    return texture;
}

int load_animation_frames(SDL_Texture** textures, const char* data_dir, const char* rel_path, int start, int end) {
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

Animation load_animation_sequence(const char* data_dir, const char* rel_path, int start, int end) {
    Animation anim;
    anim.frame_count = end - start + 1;
    anim.frames = (SDL_Texture**)malloc(anim.frame_count * sizeof(SDL_Texture*));
    if (!anim.frames) {
        anim.frame_count = 0;
        anim.sync_data = NULL;
        anim.sync_count = 0;
        return anim;
    }
    
    int loaded = 0;
    for (int i = start; i <= end; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s%s_%d.png", data_dir, rel_path, i);
        anim.frames[i - start] = load_texture(path);
        if (anim.frames[i - start]) {
            loaded++;
        }
    }
    
    anim.sync_data = NULL;
    anim.sync_count = 0;
    return anim;
}

void init_textures(const char *data_dir) {
    // Initialize all to NULL
    memset(&textures, 0, sizeof(GameTextures));
    
    char path[512];
    int loaded_count = 0;
    
    // Load fixed assets (these should always be available)
    textures.instruction_screen = load_texture("../game/resources/images/instruction_Forest.png");
    textures.bg_gradient = load_texture("../game/resources/fixed_assets/gradient.bmp");
    if (textures.instruction_screen) loaded_count++;
    if (textures.bg_gradient) loaded_count++;
    
    // Load arrow buttons
    for (int i = 0; i < 4; i++) {
        snprintf(path, sizeof(path), "../game/resources/fixed_assets/arrows.cgf_%d.png", i);
        textures.arrows[i] = load_texture(path);
        if (textures.arrows[i]) loaded_count++;
    }
    
    // Load background layers from ForestData
    snprintf(path, sizeof(path), "%s/ForestData/gfx/hillsday.cgf_0.png", data_dir);
    textures.bg_hillsday = load_texture(path);
    if (textures.bg_hillsday) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/paratrees.cgf_0.png", data_dir);
    textures.bg_trees = load_texture(path);
    if (textures.bg_trees) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/paraground.cgf_0.png", data_dir);
    textures.bg_ground = load_texture(path);
    if (textures.bg_ground) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/GRASS.cgf_0.png", data_dir);
    textures.grass = load_texture(path);
    if (textures.grass) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/LEAVES1.cgf_0.png", data_dir);
    textures.leaves1 = load_texture(path);
    if (textures.leaves1) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/LEAVES2.cgf_0.png", data_dir);
    textures.leaves2 = load_texture(path);
    if (textures.leaves2) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/SCOREBRD.bmp", data_dir);
    textures.scoreboard = load_texture(path);
    if (textures.scoreboard) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/WALL.cgf_0.png", data_dir);
    textures.end_mountain = load_texture(path);
    if (textures.end_mountain) loaded_count++;
    
    // Load Hugo animations
    loaded_count += load_animation_frames(textures.hugo_side, 
        data_dir, "/ForestData/gfx/hugoside.cgf", 0, 7);
    loaded_count += load_animation_frames(textures.hugo_jump, 
        data_dir, "/ForestData/gfx/hugohop.cgf", 0, 2);
    loaded_count += load_animation_frames(textures.hugo_crawl, 
        data_dir, "/ForestData/gfx/kravle.cgf", 0, 7);
    loaded_count += load_animation_frames(textures.hugo_telllives, 
        data_dir, "/ForestData/gfx/hugo_hello.cgf", 0, 15);
    
    // Load obstacles
    loaded_count += load_animation_frames(textures.catapult, 
        data_dir, "/ForestData/gfx/catapult.cgf", 0, 7);
    loaded_count += load_animation_frames(textures.trap, 
        data_dir, "/ForestData/gfx/faelde.cgf", 0, 5);
    loaded_count += load_animation_frames(textures.rock, 
        data_dir, "/ForestData/gfx/stone.cgf", 0, 7);
    loaded_count += load_animation_frames(textures.tree, 
        data_dir, "/ForestData/gfx/branch-swing.cgf", 0, 6);
    loaded_count += load_animation_frames(textures.sack, 
        data_dir, "/ForestData/gfx/saek.cgf", 0, 3);
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/lonetree.cgf_0.png", data_dir);
    textures.lone_tree = load_texture(path);
    if (textures.lone_tree) loaded_count++;
    
    // Load HUD elements
    snprintf(path, sizeof(path), "%s/ForestData/gfx/SCORES.cgf_0.png", data_dir);
    textures.score_numbers = load_texture(path);
    if (textures.score_numbers) loaded_count++;
    
    snprintf(path, sizeof(path), "%s/ForestData/gfx/HUGOSTAT.cgf_0.png", data_dir);
    textures.hugo_lives = load_texture(path);
    if (textures.hugo_lives) loaded_count++;
    
    // Load forest hurt animation sequences
    textures.hugohitlog = load_animation_sequence(data_dir, "/ForestData/gfx/BRANCH-GROGGY.til", 0, 42);
    textures.hugohitlog_talk = load_animation_sequence(data_dir, "/ForestData/gfx/BRANCH-SPEAK.til", 0, 17);
    textures.catapult_fly = load_animation_sequence(data_dir, "/ForestData/gfx/HGKATFLY.til", 0, 113);
    textures.catapult_fall = load_animation_sequence(data_dir, "/ForestData/gfx/HGKATFLY.til", 115, 189);
    textures.catapult_airtalk = load_animation_sequence(data_dir, "/ForestData/gfx/CATAPULT-SPEAK.til", 0, 15);
    textures.catapult_hang = load_animation_sequence(data_dir, "/ForestData/gfx/HGKATHNG.TIL", 0, 12);
    textures.catapult_hangspeak = load_animation_sequence(data_dir, "/ForestData/gfx/hanging_mouth.cgf", 0, 11);
    textures.hugo_lookrock = load_animation_sequence(data_dir, "/ForestData/gfx/hugo-rock.til", 0, 14);
    textures.hit_rock = load_animation_sequence(data_dir, "/ForestData/gfx/HGROCK.TIL", 0, 60);
    textures.hit_rock_sync = load_animation_sequence(data_dir, "/ForestData/gfx/MSYNCRCK.TIL", 0, 17);
    textures.hugo_traphurt = load_animation_sequence(data_dir, "/ForestData/gfx/TRAP-HURTS.til", 0, 9);
    textures.hugo_traptalk = load_animation_sequence(data_dir, "/ForestData/gfx/traptalk.til", 0, 15);
    
    // Load cave animation sequences
    textures.cave_talks = load_animation_sequence(data_dir, "/RopeOutroData/gfx/STAIRS.TIL", 0, 12);
    textures.cave_climbs = load_animation_sequence(data_dir, "/RopeOutroData/gfx/STAIRS.TIL", 11, 51);
    textures.cave_first_rope = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 0, 32);
    textures.cave_second_rope = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 33, 72);
    textures.cave_third_rope = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 73, 121);
    textures.cave_scylla_leaves = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 122, 177);
    textures.cave_scylla_bird = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 178, 240);
    textures.cave_scylla_ropes = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 241, 283);
    textures.cave_scylla_spring = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 284, 318);
    textures.cave_family_cage = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 319, 352);
    textures.cave_hugo_puff_first = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASEDIE.TIL", 122, 166);
    textures.cave_hugo_puff_second = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASEDIE.TIL", 167, 211);
    textures.cave_hugo_puff_third = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASEDIE.TIL", 212, 256);
    textures.cave_hugo_spring = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASEDIE.TIL", 257, 295);
    textures.cave_happy = load_animation_sequence(data_dir, "/RopeOutroData/gfx/HAPPY.TIL", 0, 111);
    
    // Load cave hugo sprite
    snprintf(path, sizeof(path), "%s/RopeOutroData/gfx/hugo.cgf_0.png", data_dir);
    textures.cave_hugo_sprite = load_texture(path);
    
    printf("Loaded %d textures (data_dir: %s)\n", loaded_count, data_dir);
}

void free_textures() {
    // Free single textures
    if (textures.instruction_screen) SDL_DestroyTexture(textures.instruction_screen);
    if (textures.bg_gradient) SDL_DestroyTexture(textures.bg_gradient);
    if (textures.bg_hillsday) SDL_DestroyTexture(textures.bg_hillsday);
    if (textures.bg_trees) SDL_DestroyTexture(textures.bg_trees);
    if (textures.bg_ground) SDL_DestroyTexture(textures.bg_ground);
    if (textures.grass) SDL_DestroyTexture(textures.grass);
    if (textures.leaves1) SDL_DestroyTexture(textures.leaves1);
    if (textures.leaves2) SDL_DestroyTexture(textures.leaves2);
    if (textures.end_mountain) SDL_DestroyTexture(textures.end_mountain);
    if (textures.scoreboard) SDL_DestroyTexture(textures.scoreboard);
    if (textures.score_numbers) SDL_DestroyTexture(textures.score_numbers);
    if (textures.lone_tree) SDL_DestroyTexture(textures.lone_tree);
    if (textures.hugo_lives) SDL_DestroyTexture(textures.hugo_lives);
    
    // Free arrays
    for (int i = 0; i < 4; i++) {
        if (textures.arrows[i]) SDL_DestroyTexture(textures.arrows[i]);
        if (textures.sack[i]) SDL_DestroyTexture(textures.sack[i]);
    }
    for (int i = 0; i < 3; i++) {
        if (textures.hugo_jump[i]) SDL_DestroyTexture(textures.hugo_jump[i]);
    }
    for (int i = 0; i < 6; i++) {
        if (textures.trap[i]) SDL_DestroyTexture(textures.trap[i]);
    }
    for (int i = 0; i < 7; i++) {
        if (textures.tree[i]) SDL_DestroyTexture(textures.tree[i]);
    }
    for (int i = 0; i < 8; i++) {
        if (textures.hugo_side[i]) SDL_DestroyTexture(textures.hugo_side[i]);
        if (textures.hugo_crawl[i]) SDL_DestroyTexture(textures.hugo_crawl[i]);
        if (textures.catapult[i]) SDL_DestroyTexture(textures.catapult[i]);
        if (textures.rock[i]) SDL_DestroyTexture(textures.rock[i]);
    }
    for (int i = 0; i < 16; i++) {
        if (textures.hugo_telllives[i]) SDL_DestroyTexture(textures.hugo_telllives[i]);
    }
    
    // Helper macro to free animation sequences
    #define FREE_ANIM(anim) \
        if (anim.frames) { \
            for (int i = 0; i < anim.frame_count; i++) { \
                if (anim.frames[i]) SDL_DestroyTexture(anim.frames[i]); \
            } \
            free(anim.frames); \
        } \
        if (anim.sync_data) { \
            free(anim.sync_data); \
        }
    
    // Free forest hurt animations
    FREE_ANIM(textures.hugohitlog);
    FREE_ANIM(textures.hugohitlog_talk);
    FREE_ANIM(textures.catapult_fly);
    FREE_ANIM(textures.catapult_fall);
    FREE_ANIM(textures.catapult_airtalk);
    FREE_ANIM(textures.catapult_hang);
    FREE_ANIM(textures.catapult_hangspeak);
    FREE_ANIM(textures.hugo_lookrock);
    FREE_ANIM(textures.hit_rock);
    FREE_ANIM(textures.hit_rock_sync);
    FREE_ANIM(textures.hugo_traphurt);
    FREE_ANIM(textures.hugo_traptalk);
    
    // Free cave animations
    FREE_ANIM(textures.cave_talks);
    FREE_ANIM(textures.cave_climbs);
    FREE_ANIM(textures.cave_first_rope);
    FREE_ANIM(textures.cave_second_rope);
    FREE_ANIM(textures.cave_third_rope);
    FREE_ANIM(textures.cave_scylla_bird);
    FREE_ANIM(textures.cave_scylla_leaves);
    FREE_ANIM(textures.cave_scylla_ropes);
    FREE_ANIM(textures.cave_scylla_spring);
    FREE_ANIM(textures.cave_hugo_puff_first);
    FREE_ANIM(textures.cave_hugo_puff_second);
    FREE_ANIM(textures.cave_hugo_puff_third);
    FREE_ANIM(textures.cave_hugo_spring);
    FREE_ANIM(textures.cave_family_cage);
    FREE_ANIM(textures.cave_happy);
    
    if (textures.cave_hugo_sprite) SDL_DestroyTexture(textures.cave_hugo_sprite);
    
    #undef FREE_ANIM
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

void free_audio() {
    // Free forest speech
    if (audio.speak_start) Mix_FreeChunk(audio.speak_start);
    if (audio.speak_rock) Mix_FreeChunk(audio.speak_rock);
    if (audio.speak_dieonce) Mix_FreeChunk(audio.speak_dieonce);
    if (audio.speak_trap) Mix_FreeChunk(audio.speak_trap);
    if (audio.speak_lastlife) Mix_FreeChunk(audio.speak_lastlife);
    if (audio.speak_catapult_up) Mix_FreeChunk(audio.speak_catapult_up);
    if (audio.speak_catapult_hit) Mix_FreeChunk(audio.speak_catapult_hit);
    if (audio.speak_catapult_talktop) Mix_FreeChunk(audio.speak_catapult_talktop);
    if (audio.speak_catapult_down) Mix_FreeChunk(audio.speak_catapult_down);
    if (audio.speak_catapult_hang) Mix_FreeChunk(audio.speak_catapult_hang);
    if (audio.speak_hitlog) Mix_FreeChunk(audio.speak_hitlog);
    if (audio.speak_gameover) Mix_FreeChunk(audio.speak_gameover);
    if (audio.speak_levelcompleted) Mix_FreeChunk(audio.speak_levelcompleted);
    
    // Free forest sound effects
    if (audio.sfx_bg_atmosphere) Mix_FreeChunk(audio.sfx_bg_atmosphere);
    if (audio.sfx_lightning_warning) Mix_FreeChunk(audio.sfx_lightning_warning);
    if (audio.sfx_hugo_knock) Mix_FreeChunk(audio.sfx_hugo_knock);
    if (audio.sfx_hugo_hittrap) Mix_FreeChunk(audio.sfx_hugo_hittrap);
    if (audio.sfx_hugo_launch) Mix_FreeChunk(audio.sfx_hugo_launch);
    if (audio.sfx_sack_normal) Mix_FreeChunk(audio.sfx_sack_normal);
    if (audio.sfx_sack_bonus) Mix_FreeChunk(audio.sfx_sack_bonus);
    if (audio.sfx_tree_swush) Mix_FreeChunk(audio.sfx_tree_swush);
    if (audio.sfx_hugo_hitlog) Mix_FreeChunk(audio.sfx_hugo_hitlog);
    if (audio.sfx_catapult_eject) Mix_FreeChunk(audio.sfx_catapult_eject);
    if (audio.sfx_birds) Mix_FreeChunk(audio.sfx_birds);
    if (audio.sfx_hugo_hitscreen) Mix_FreeChunk(audio.sfx_hugo_hitscreen);
    if (audio.sfx_hugo_screenklir) Mix_FreeChunk(audio.sfx_hugo_screenklir);
    if (audio.sfx_hugo_crash) Mix_FreeChunk(audio.sfx_hugo_crash);
    if (audio.sfx_hugo_hangstart) Mix_FreeChunk(audio.sfx_hugo_hangstart);
    if (audio.sfx_hugo_hang) Mix_FreeChunk(audio.sfx_hugo_hang);
    
    for (int i = 0; i < 5; i++) {
        if (audio.sfx_hugo_walk[i]) Mix_FreeChunk(audio.sfx_hugo_walk[i]);
    }
    
    // Free cave speech
    if (audio.cave_her_er_vi) Mix_FreeChunk(audio.cave_her_er_vi);
    if (audio.cave_trappe_snak) Mix_FreeChunk(audio.cave_trappe_snak);
    if (audio.cave_nu_kommer_jeg) Mix_FreeChunk(audio.cave_nu_kommer_jeg);
    if (audio.cave_afskylia_snak) Mix_FreeChunk(audio.cave_afskylia_snak);
    if (audio.cave_hugo_katapult) Mix_FreeChunk(audio.cave_hugo_katapult);
    if (audio.cave_hugo_skyd_ud) Mix_FreeChunk(audio.cave_hugo_skyd_ud);
    if (audio.cave_afskylia_skyd_ud) Mix_FreeChunk(audio.cave_afskylia_skyd_ud);
    if (audio.cave_hugoline_tak) Mix_FreeChunk(audio.cave_hugoline_tak);
    
    // Free cave sound effects
    if (audio.cave_stemning) Mix_FreeChunk(audio.cave_stemning);
    if (audio.cave_fodtrin1) Mix_FreeChunk(audio.cave_fodtrin1);
    if (audio.cave_fodtrin2) Mix_FreeChunk(audio.cave_fodtrin2);
    if (audio.cave_hiv_i_reb) Mix_FreeChunk(audio.cave_hiv_i_reb);
    if (audio.cave_fjeder) Mix_FreeChunk(audio.cave_fjeder);
    if (audio.cave_pre_puf) Mix_FreeChunk(audio.cave_pre_puf);
    if (audio.cave_puf) Mix_FreeChunk(audio.cave_puf);
    if (audio.cave_tast_trykket) Mix_FreeChunk(audio.cave_tast_trykket);
    if (audio.cave_pre_fanfare) Mix_FreeChunk(audio.cave_pre_fanfare);
    if (audio.cave_fanfare) Mix_FreeChunk(audio.cave_fanfare);
    if (audio.cave_fugle_skrig) Mix_FreeChunk(audio.cave_fugle_skrig);
    if (audio.cave_trappe_grin) Mix_FreeChunk(audio.cave_trappe_grin);
    if (audio.cave_skrig) Mix_FreeChunk(audio.cave_skrig);
    if (audio.cave_score_counter) Mix_FreeChunk(audio.cave_score_counter);
    
}

bool init_sdl(const char *data_dir) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Hugo",
                             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE,
                             SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    
    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    // Initialize SDL_mixer (optional)
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Warning: SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    Mix_AllocateChannels(16);
    
    // Load game textures
    init_textures(data_dir);
    
    // Load game audio
    init_audio(data_dir);

    return true;
}

// Main game loop
void game_loop() {
    bool quit = false;
    SDL_Event event;
    
    // Initialize
    init_game_context();
    state_info.current_state = STATE_INSTRUCTIONS;
    state_info.state_start_time = SDL_GetTicks() / 1000.0;
    srand(time(NULL));
    InputState input_state = {0};
    
    while (!quit && state_info.current_state != STATE_END) {
        // Handle events

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (event.key.keysym.sym == SDLK_3) {
                    input_state.cave_rope_1_pressed = true;
                } else if (event.key.keysym.sym == SDLK_6) {
                    input_state.cave_rope_2_pressed = true;
                } else if (event.key.keysym.sym == SDLK_9) {
                    input_state.cave_rope_3_pressed = true;
                } else if (event.key.keysym.sym == SDLK_2 || event.key.keysym.sym == SDLK_UP) {
                    input_state.key_up = true;
                } else if (event.key.keysym.sym == SDLK_8 || event.key.keysym.sym == SDLK_DOWN) {
                    input_state.key_down = true;
                } else if (event.key.keysym.sym == SDLK_5) {
                    input_state.key_start = true;
                }
            } else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_3) {
                    input_state.cave_rope_1_pressed = false;
                } else if (event.key.keysym.sym == SDLK_6) {
                    input_state.cave_rope_2_pressed = false;
                } else if (event.key.keysym.sym == SDLK_9) {
                    input_state.cave_rope_3_pressed = false;
                } else if (event.key.keysym.sym == SDLK_2 || event.key.keysym.sym == SDLK_UP) {
                    input_state.key_up = false;
                } else if (event.key.keysym.sym == SDLK_8 || event.key.keysym.sym == SDLK_DOWN) {
                    input_state.key_down = false;
                } else if (event.key.keysym.sym == SDLK_5) {
                    input_state.key_start = false;
                }
            }
        }
        
        GameState new_state = process_funcs[state_info.current_state](input_state);

        if(new_state != STATE_NONE) {
            printf("State transition: %s -> %s\n", game_state_name[state_info.current_state], game_state_name[new_state]);
            state_info.current_state = new_state;
            state_info.state_start_time = SDL_GetTicks() / 1000.0;
        }

        render_funcs[state_info.current_state]();
        SDL_RenderPresent(renderer);

        SDL_Delay(1000 / 30);
    }
    
    printf("Game ended. Final score: %d\n", game_ctx.score);
}

void cleanup() {
    // Free audio
    free_audio();
    
    // Free textures
    free_textures();
    
    Mix_CloseAudio();
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    
    if (window) {
        SDL_DestroyWindow(window);
    }
    
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <decompressed_data_directory>\n", argv[0]);
        return 1;
    }
    
    printf("Hugo Lite - Single Player Forest Game\n");
    printf("Based on the Hugo TV game from the 90s\n");
    printf("Controls: Press 2/UP to JUMP, 8/DOWN to DUCK, 5 to START, ESC to quit\n\n");
    
    if (!init_sdl(argv[1])) {
        printf("Failed to initialize!\n");
        return 1;
    }
    
    game_loop();
    cleanup();
    
    return 0;
}
