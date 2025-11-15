/*
 * Hugo Lite - Single Player C/SDL2 Version
 * A simplified version of the Hugo forest and cave mini games
 * Based on the Python implementation in ../game/
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Screen dimensions
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define WINDOW_SCALE 2

// Game constants
#define FOREST_MAX_TIME 60
#define FOREST_GROUND_SPEED 75
#define HUGO_X_POS 16
#define START_LIVES 3

typedef enum {
    STATE_INSTRUCTIONS,

    // Forest states
    STATE_FOREST_BRANCH_ANIMATION,
    STATE_FOREST_BRANCH_TALKING,
    STATE_FOREST_FLYING_FALLING,
    STATE_FOREST_FLYING_FALLING_HANG_ANIMATION,
    STATE_FOREST_FLYING_FALLING_HANG_TALKING,
    STATE_FOREST_FLYING_START,
    STATE_FOREST_FLYING_TALKING,
    STATE_FOREST_ROCK_ANIMATION,
    STATE_FOREST_ROCK_HIT_ANIMATION,
    STATE_FOREST_ROCK_TALKING,
    STATE_FOREST_TRAP_ANIMATION,
    STATE_FOREST_TRAP_TALKING,
    STATE_FOREST_PLAYING,
    STATE_FOREST_SCYLLA_BUTTON,
    STATE_FOREST_TALKING_AFTER_HURT,
    STATE_FOREST_TALKING_GAME_OVER,
    STATE_FOREST_WAIT_INTRO,
    STATE_FOREST_WIN_TALKING,

    // Cave bonus game states
    STATE_CAVE_CLIMBING,
    STATE_CAVE_FAMILY_CAGE_OPENS,
    STATE_CAVE_FAMILY_HAPPY,
    STATE_CAVE_GOING_ROPE,
    STATE_CAVE_LOST,
    STATE_CAVE_LOST_SPRING,
    STATE_CAVE_SCYLLA_LOST,
    STATE_CAVE_SCYLLA_SPRING,
    STATE_CAVE_TALKING_BEFORE_CLIMB,
    STATE_CAVE_WAITING_BEFORE_TALKING,
    STATE_CAVE_WAITING_INPUT,

    // End state
    STATE_END
} GameState;

typedef enum {
    OBS_NONE = 0,
    OBS_CATAPULT = 1,
    OBS_TRAP = 2,
    OBS_ROCK = 3,
    OBS_TREE = 4
} ObstacleType;

typedef struct {
    int score;
    int lives;
    double parallax_pos;
    ObstacleType obstacles[FOREST_MAX_TIME];
    int sacks[FOREST_MAX_TIME];
    bool arrow_up_focus;
    bool arrow_down_focus;
    double hugo_jumping_time;
    double hugo_crawling_time;
    double last_time;
    int old_second;
    // Cave game data
    int cave_selected_rope;  // 0, 1, or 2 for ropes 3, 6, 9
    int cave_win_type;       // 0=bird, 1=leaves, 2=ropes
    int rolling_score;
} GameContext;

typedef struct {
    double state_start_time;
    GameState current_state;
} StateInfo;

typedef struct {
    // Fixed assets
    SDL_Texture* instruction_screen;
    SDL_Texture* arrows[4];  // up, up_pressed, down, down_pressed
    SDL_Texture* bg_gradient;
    
    // Background layers
    SDL_Texture* bg_hillsday;
    SDL_Texture* bg_trees;
    SDL_Texture* bg_ground;
    SDL_Texture* grass;
    SDL_Texture* leaves1;
    SDL_Texture* leaves2;
    SDL_Texture* end_mountain;
    
    // Hugo animations
    SDL_Texture* hugo_side[8];     // walking (0-7)
    SDL_Texture* hugo_jump[3];     // jumping (0-2)
    SDL_Texture* hugo_crawl[8];    // crawling (0-7)
    SDL_Texture* hugo_telllives[16]; // intro animation (0-15)
    
    // Obstacles
    SDL_Texture* catapult[8];      // catapult (0-7)
    SDL_Texture* trap[6];          // trap (0-5)
    SDL_Texture* rock[8];          // rolling rock (0-7)
    SDL_Texture* tree[7];          // branch swinging (0-6)
    SDL_Texture* lone_tree;        // tree trunk
    SDL_Texture* sack[4];          // sacks (0-3)
    
    // HUD elements
    SDL_Texture* scoreboard;
    SDL_Texture* score_numbers;    // score digits spritesheet
    SDL_Texture* hugo_lives;       // life indicator
    
    // Forest hurt animations
    SDL_Texture** hugohitlog;          // branch hit animation (0-42)
    int hugohitlog_count;
    SDL_Texture** hugohitlog_talk;     // branch talking (0-17)
    int hugohitlog_talk_count;
    SDL_Texture** catapult_fly;        // flying up (0-113)
    int catapult_fly_count;
    SDL_Texture** catapult_fall;       // falling down (0-74)
    int catapult_fall_count;
    SDL_Texture** catapult_airtalk;    // talking in air (0-15)
    int catapult_airtalk_count;
    SDL_Texture** catapult_hang;       // hanging (0-12)
    int catapult_hang_count;
    SDL_Texture** catapult_hangspeak;  // hanging mouth (0-11)
    int catapult_hangspeak_count;
    SDL_Texture** hugo_lookrock;       // looking at rock (0-14)
    int hugo_lookrock_count;
    SDL_Texture** hit_rock;            // hit by rock (0-60)
    int hit_rock_count;
    SDL_Texture** hit_rock_sync;       // rock talking (0-17)
    int hit_rock_sync_count;
    SDL_Texture** hugo_traphurt;       // trap hurt (0-9)
    int hugo_traphurt_count;
    SDL_Texture** hugo_traptalk;       // trap talking (0-15)
    int hugo_traptalk_count;
    
    // Cave animations
    SDL_Texture** cave_talks;          // cave intro talking (0-12)
    int cave_talks_count;
    SDL_Texture** cave_climbs;         // climbing (0-40)
    int cave_climbs_count;
    SDL_Texture** cave_first_rope;     // first rope descent (0-32)
    int cave_first_rope_count;
    SDL_Texture** cave_second_rope;    // second rope descent (0-39)
    int cave_second_rope_count;
    SDL_Texture** cave_third_rope;     // third rope descent (0-48)
    int cave_third_rope_count;
    SDL_Texture** cave_scylla_bird;    // scylla bird (0-62)
    int cave_scylla_bird_count;
    SDL_Texture** cave_scylla_leaves;  // scylla leaves (0-55)
    int cave_scylla_leaves_count;
    SDL_Texture** cave_scylla_ropes;   // scylla ropes (0-42)
    int cave_scylla_ropes_count;
    SDL_Texture** cave_scylla_spring;  // scylla spring (0-34)
    int cave_scylla_spring_count;
    SDL_Texture** cave_hugo_puff_first;  // hugo puff first rope (0-44)
    int cave_hugo_puff_first_count;
    SDL_Texture** cave_hugo_puff_second; // hugo puff second rope (0-44)
    int cave_hugo_puff_second_count;
    SDL_Texture** cave_hugo_puff_third;  // hugo puff third rope (0-44)
    int cave_hugo_puff_third_count;
    SDL_Texture** cave_hugo_spring;    // hugo spring (0-38)
    int cave_hugo_spring_count;
    SDL_Texture** cave_family_cage;    // family cage opens (0-33)
    int cave_family_cage_count;
    SDL_Texture** cave_happy;          // happy ending (0-111)
    int cave_happy_count;
    SDL_Texture* cave_hugo_sprite;     // hugo standing sprite
} GameTextures;

typedef struct {
    // Forest speech
    Mix_Chunk* speak_start;
    Mix_Chunk* speak_rock;
    Mix_Chunk* speak_dieonce;
    Mix_Chunk* speak_trap;
    Mix_Chunk* speak_lastlife;
    Mix_Chunk* speak_catapult_up;
    Mix_Chunk* speak_catapult_hit;
    Mix_Chunk* speak_catapult_talktop;
    Mix_Chunk* speak_catapult_down;
    Mix_Chunk* speak_catapult_hang;
    Mix_Chunk* speak_hitlog;
    Mix_Chunk* speak_gameover;
    Mix_Chunk* speak_levelcompleted;
    
    // Forest sound effects
    Mix_Chunk* sfx_bg_atmosphere;
    Mix_Chunk* sfx_lightning_warning;
    Mix_Chunk* sfx_hugo_knock;
    Mix_Chunk* sfx_hugo_hittrap;
    Mix_Chunk* sfx_hugo_launch;
    Mix_Chunk* sfx_sack_normal;
    Mix_Chunk* sfx_sack_bonus;
    Mix_Chunk* sfx_tree_swush;
    Mix_Chunk* sfx_hugo_hitlog;
    Mix_Chunk* sfx_catapult_eject;
    Mix_Chunk* sfx_birds;
    Mix_Chunk* sfx_hugo_hitscreen;
    Mix_Chunk* sfx_hugo_screenklir;
    Mix_Chunk* sfx_hugo_crash;
    Mix_Chunk* sfx_hugo_hangstart;
    Mix_Chunk* sfx_hugo_hang;
    Mix_Chunk* sfx_hugo_walk[5];
    
    // Cave speech
    Mix_Chunk* cave_her_er_vi;
    Mix_Chunk* cave_trappe_snak;
    Mix_Chunk* cave_nu_kommer_jeg;
    Mix_Chunk* cave_afskylia_snak;
    Mix_Chunk* cave_hugo_katapult;
    Mix_Chunk* cave_hugo_skyd_ud;
    Mix_Chunk* cave_afskylia_skyd_ud;
    Mix_Chunk* cave_hugoline_tak;
    
    // Cave sound effects
    Mix_Chunk* cave_stemning;
    Mix_Chunk* cave_fodtrin1;
    Mix_Chunk* cave_fodtrin2;
    Mix_Chunk* cave_hiv_i_reb;
    Mix_Chunk* cave_fjeder;
    Mix_Chunk* cave_pre_puf;
    Mix_Chunk* cave_puf;
    Mix_Chunk* cave_tast_trykket;
    Mix_Chunk* cave_pre_fanfare;
    Mix_Chunk* cave_fanfare;
    Mix_Chunk* cave_fugle_skrig;
    Mix_Chunk* cave_trappe_grin;
    Mix_Chunk* cave_skrig;
    Mix_Chunk* cave_score_counter;
} GameAudio;

typedef struct {
    bool key_up;
    bool key_down;
    bool key_start;
    bool cave_rope_1_pressed;
    bool cave_rope_2_pressed;
    bool cave_rope_3_pressed;
} InputState;

const char * game_state_name[] = {
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

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameContext game_ctx;
StateInfo state_info;
GameTextures textures;
GameAudio audio;

double get_state_time() {
    return (SDL_GetTicks() / 1000.0) - state_info.state_start_time;
}

int get_frame_index() {
    return (int)(get_state_time() * 10);
}

int get_frame_index_fast() {
    return (int)(get_state_time() * 20);
}

double get_game_time() {
    return SDL_GetTicks() / 1000.0;
}

SDL_Texture* load_texture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Warning: Could not load image %s: %s\n", path, IMG_GetError());
        return NULL;
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

SDL_Texture** load_animation_sequence(const char* data_dir, const char* rel_path, int start, int end, int* out_count) {
    int count = end - start + 1;
    SDL_Texture** textures_arr = (SDL_Texture**)malloc(count * sizeof(SDL_Texture*));
    if (!textures_arr) {
        *out_count = 0;
        return NULL;
    }
    
    int loaded = 0;
    for (int i = start; i <= end; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s%s_%d.png", data_dir, rel_path, i);
        textures_arr[i - start] = load_texture(path);
        if (textures_arr[i - start]) {
            loaded++;
        }
    }
    
    *out_count = count;
    return textures_arr;
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
    textures.hugohitlog = load_animation_sequence(data_dir, "/ForestData/gfx/BRANCH-GROGGY.til", 0, 42, &textures.hugohitlog_count);
    textures.hugohitlog_talk = load_animation_sequence(data_dir, "/ForestData/gfx/BRANCH-SPEAK.til", 0, 17, &textures.hugohitlog_talk_count);
    textures.catapult_fly = load_animation_sequence(data_dir, "/ForestData/gfx/HGKATFLY.til", 0, 113, &textures.catapult_fly_count);
    textures.catapult_fall = load_animation_sequence(data_dir, "/ForestData/gfx/HGKATFLY.til", 115, 189, &textures.catapult_fall_count);
    textures.catapult_airtalk = load_animation_sequence(data_dir, "/ForestData/gfx/CATAPULT-SPEAK.til", 0, 15, &textures.catapult_airtalk_count);
    textures.catapult_hang = load_animation_sequence(data_dir, "/ForestData/gfx/HGKATHNG.TIL", 0, 12, &textures.catapult_hang_count);
    textures.catapult_hangspeak = load_animation_sequence(data_dir, "/ForestData/gfx/hanging_mouth.cgf", 0, 11, &textures.catapult_hangspeak_count);
    textures.hugo_lookrock = load_animation_sequence(data_dir, "/ForestData/gfx/hugo-rock.til", 0, 14, &textures.hugo_lookrock_count);
    textures.hit_rock = load_animation_sequence(data_dir, "/ForestData/gfx/HGROCK.TIL", 0, 60, &textures.hit_rock_count);
    textures.hit_rock_sync = load_animation_sequence(data_dir, "/ForestData/gfx/MSYNCRCK.TIL", 0, 17, &textures.hit_rock_sync_count);
    textures.hugo_traphurt = load_animation_sequence(data_dir, "/ForestData/gfx/TRAP-HURTS.til", 0, 9, &textures.hugo_traphurt_count);
    textures.hugo_traptalk = load_animation_sequence(data_dir, "/ForestData/gfx/traptalk.til", 0, 15, &textures.hugo_traptalk_count);
    
    // Load cave animation sequences
    textures.cave_talks = load_animation_sequence(data_dir, "/RopeOutroData/gfx/STAIRS.TIL", 0, 12, &textures.cave_talks_count);
    textures.cave_climbs = load_animation_sequence(data_dir, "/RopeOutroData/gfx/STAIRS.TIL", 11, 51, &textures.cave_climbs_count);
    textures.cave_first_rope = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 0, 32, &textures.cave_first_rope_count);
    textures.cave_second_rope = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 33, 72, &textures.cave_second_rope_count);
    textures.cave_third_rope = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 73, 121, &textures.cave_third_rope_count);
    textures.cave_scylla_leaves = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 122, 177, &textures.cave_scylla_leaves_count);
    textures.cave_scylla_bird = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 178, 240, &textures.cave_scylla_bird_count);
    textures.cave_scylla_ropes = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 241, 283, &textures.cave_scylla_ropes_count);
    textures.cave_scylla_spring = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 284, 318, &textures.cave_scylla_spring_count);
    textures.cave_family_cage = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASELIVE.TIL", 319, 352, &textures.cave_family_cage_count);
    textures.cave_hugo_puff_first = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASEDIE.TIL", 122, 166, &textures.cave_hugo_puff_first_count);
    textures.cave_hugo_puff_second = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASEDIE.TIL", 167, 211, &textures.cave_hugo_puff_second_count);
    textures.cave_hugo_puff_third = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASEDIE.TIL", 212, 256, &textures.cave_hugo_puff_third_count);
    textures.cave_hugo_spring = load_animation_sequence(data_dir, "/RopeOutroData/gfx/CASEDIE.TIL", 257, 295, &textures.cave_hugo_spring_count);
    textures.cave_happy = load_animation_sequence(data_dir, "/RopeOutroData/gfx/HAPPY.TIL", 0, 111, &textures.cave_happy_count);
    
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
    #define FREE_ANIM_SEQ(seq, count) \
        if (seq) { \
            for (int i = 0; i < count; i++) { \
                if (seq[i]) SDL_DestroyTexture(seq[i]); \
            } \
            free(seq); \
        }
    
    // Free forest hurt animations
    FREE_ANIM_SEQ(textures.hugohitlog, textures.hugohitlog_count);
    FREE_ANIM_SEQ(textures.hugohitlog_talk, textures.hugohitlog_talk_count);
    FREE_ANIM_SEQ(textures.catapult_fly, textures.catapult_fly_count);
    FREE_ANIM_SEQ(textures.catapult_fall, textures.catapult_fall_count);
    FREE_ANIM_SEQ(textures.catapult_airtalk, textures.catapult_airtalk_count);
    FREE_ANIM_SEQ(textures.catapult_hang, textures.catapult_hang_count);
    FREE_ANIM_SEQ(textures.catapult_hangspeak, textures.catapult_hangspeak_count);
    FREE_ANIM_SEQ(textures.hugo_lookrock, textures.hugo_lookrock_count);
    FREE_ANIM_SEQ(textures.hit_rock, textures.hit_rock_count);
    FREE_ANIM_SEQ(textures.hit_rock_sync, textures.hit_rock_sync_count);
    FREE_ANIM_SEQ(textures.hugo_traphurt, textures.hugo_traphurt_count);
    FREE_ANIM_SEQ(textures.hugo_traptalk, textures.hugo_traptalk_count);
    
    // Free cave animations
    FREE_ANIM_SEQ(textures.cave_talks, textures.cave_talks_count);
    FREE_ANIM_SEQ(textures.cave_climbs, textures.cave_climbs_count);
    FREE_ANIM_SEQ(textures.cave_first_rope, textures.cave_first_rope_count);
    FREE_ANIM_SEQ(textures.cave_second_rope, textures.cave_second_rope_count);
    FREE_ANIM_SEQ(textures.cave_third_rope, textures.cave_third_rope_count);
    FREE_ANIM_SEQ(textures.cave_scylla_bird, textures.cave_scylla_bird_count);
    FREE_ANIM_SEQ(textures.cave_scylla_leaves, textures.cave_scylla_leaves_count);
    FREE_ANIM_SEQ(textures.cave_scylla_ropes, textures.cave_scylla_ropes_count);
    FREE_ANIM_SEQ(textures.cave_scylla_spring, textures.cave_scylla_spring_count);
    FREE_ANIM_SEQ(textures.cave_hugo_puff_first, textures.cave_hugo_puff_first_count);
    FREE_ANIM_SEQ(textures.cave_hugo_puff_second, textures.cave_hugo_puff_second_count);
    FREE_ANIM_SEQ(textures.cave_hugo_puff_third, textures.cave_hugo_puff_third_count);
    FREE_ANIM_SEQ(textures.cave_hugo_spring, textures.cave_hugo_spring_count);
    FREE_ANIM_SEQ(textures.cave_family_cage, textures.cave_family_cage_count);
    FREE_ANIM_SEQ(textures.cave_happy, textures.cave_happy_count);
    
    if (textures.cave_hugo_sprite) SDL_DestroyTexture(textures.cave_hugo_sprite);
    
    #undef FREE_ANIM_SEQ
}

Mix_Chunk* load_audio(const char* path) {
    Mix_Chunk* chunk = Mix_LoadWAV(path);
    if (!chunk) {
        printf("Warning: Could not load audio %s: %s\n", path, Mix_GetError());
        return NULL;
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

void change_state(GameState new_state) {
    printf("State transition: %s -> %s\n", game_state_name[state_info.current_state], game_state_name[new_state]);
    state_info.current_state = new_state;
    state_info.state_start_time = SDL_GetTicks() / 1000.0;
}

void render_instructions() {
    SDL_RenderCopy(renderer, textures.instruction_screen, NULL, NULL);
    SDL_RenderPresent(renderer);
}

// Render intro animation
void render_wait_intro() {
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

// Render playing state
void render_playing() {
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
    
    if (textures.hugohitlog && textures.hugohitlog_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.hugohitlog_count) frame = textures.hugohitlog_count - 1;
        if (textures.hugohitlog[frame]) {
            SDL_RenderCopy(renderer, textures.hugohitlog[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_branch_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hugohitlog_talk && textures.hugohitlog_talk_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.hugohitlog_talk_count) frame = textures.hugohitlog_talk_count - 1;
        if (textures.hugohitlog_talk[frame]) {
            SDL_RenderCopy(renderer, textures.hugohitlog_talk[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_flying_start() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.catapult_fly && textures.catapult_fly_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.catapult_fly_count) frame = textures.catapult_fly_count - 1;
        if (textures.catapult_fly[frame]) {
            SDL_RenderCopy(renderer, textures.catapult_fly[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_flying_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.catapult_airtalk && textures.catapult_airtalk_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.catapult_airtalk_count) frame = textures.catapult_airtalk_count - 1;
        if (textures.catapult_airtalk[frame]) {
            SDL_RenderCopy(renderer, textures.catapult_airtalk[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_flying_falling() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.catapult_fall && textures.catapult_fall_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.catapult_fall_count) frame = textures.catapult_fall_count - 1;
        if (textures.catapult_fall[frame]) {
            SDL_RenderCopy(renderer, textures.catapult_fall[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_flying_falling_hang_animation() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.catapult_hang && textures.catapult_hang_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.catapult_hang_count) frame = textures.catapult_hang_count - 1;
        if (textures.catapult_hang[frame]) {
            SDL_RenderCopy(renderer, textures.catapult_hang[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_flying_falling_hang_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render static hang frame as background
    if (textures.catapult_hang && textures.catapult_hang_count > 12) {
        if (textures.catapult_hang[12]) {
            SDL_RenderCopy(renderer, textures.catapult_hang[12], NULL, NULL);
        }
    }
    
    // Render talking mouth animation on top
    if (textures.catapult_hangspeak && textures.catapult_hangspeak_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.catapult_hangspeak_count) frame = textures.catapult_hangspeak_count - 1;
        if (textures.catapult_hangspeak[frame]) {
            SDL_Rect mouth_pos = {115, 117, 0, 0};
            SDL_QueryTexture(textures.catapult_hangspeak[frame], NULL, NULL, &mouth_pos.w, &mouth_pos.h);
            SDL_RenderCopy(renderer, textures.catapult_hangspeak[frame], NULL, &mouth_pos);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_rock_animation() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hugo_lookrock && textures.hugo_lookrock_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.hugo_lookrock_count) frame = textures.hugo_lookrock_count - 1;
        if (textures.hugo_lookrock[frame]) {
            SDL_RenderCopy(renderer, textures.hugo_lookrock[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_rock_hit_animation() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hit_rock && textures.hit_rock_count > 0) {
        int frame = get_frame_index_fast();
        if (frame >= textures.hit_rock_count) frame = textures.hit_rock_count - 1;
        if (textures.hit_rock[frame]) {
            SDL_RenderCopy(renderer, textures.hit_rock[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_rock_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hit_rock_sync && textures.hit_rock_sync_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.hit_rock_sync_count) frame = textures.hit_rock_sync_count - 1;
        if (textures.hit_rock_sync[frame]) {
            SDL_RenderCopy(renderer, textures.hit_rock_sync[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_trap_animation() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hugo_traphurt && textures.hugo_traphurt_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.hugo_traphurt_count) frame = textures.hugo_traphurt_count - 1;
        if (textures.hugo_traphurt[frame]) {
            SDL_RenderCopy(renderer, textures.hugo_traphurt[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_trap_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.hugo_traptalk && textures.hugo_traptalk_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.hugo_traptalk_count) frame = textures.hugo_traptalk_count - 1;
        if (textures.hugo_traptalk[frame]) {
            SDL_RenderCopy(renderer, textures.hugo_traptalk[frame], NULL, NULL);
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
    
    if (textures.cave_talks && textures.cave_talks_count > 12) {
        // Show frame 12 (last frame of talks before climbing)
        if (textures.cave_talks[12]) {
            SDL_RenderCopy(renderer, textures.cave_talks[12], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_talking_before_climb() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.cave_talks && textures.cave_talks_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_talks_count) frame = textures.cave_talks_count - 1;
        if (textures.cave_talks[frame]) {
            SDL_RenderCopy(renderer, textures.cave_talks[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_climbing() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.cave_climbs && textures.cave_climbs_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_climbs_count) frame = textures.cave_climbs_count - 1;
        if (textures.cave_climbs[frame]) {
            SDL_RenderCopy(renderer, textures.cave_climbs[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_waiting_input() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Show last frame of climbing animation
    if (textures.cave_climbs && textures.cave_climbs_count > 0) {
        int last_frame = textures.cave_climbs_count - 1;
        if (textures.cave_climbs[last_frame]) {
            SDL_RenderCopy(renderer, textures.cave_climbs[last_frame], NULL, NULL);
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
        rope_animation = textures.cave_first_rope;
        rope_count = textures.cave_first_rope_count;
    } else if (game_ctx.cave_selected_rope == 1) {
        rope_animation = textures.cave_second_rope;
        rope_count = textures.cave_second_rope_count;
    } else if (game_ctx.cave_selected_rope == 2) {
        rope_animation = textures.cave_third_rope;
        rope_count = textures.cave_third_rope_count;
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
        puff_animation = textures.cave_hugo_puff_first;
        puff_count = textures.cave_hugo_puff_first_count;
    } else if (game_ctx.cave_selected_rope == 1) {
        puff_animation = textures.cave_hugo_puff_second;
        puff_count = textures.cave_hugo_puff_second_count;
    } else if (game_ctx.cave_selected_rope == 2) {
        puff_animation = textures.cave_hugo_puff_third;
        puff_count = textures.cave_hugo_puff_third_count;
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
    
    if (textures.cave_hugo_spring && textures.cave_hugo_spring_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_hugo_spring_count) frame = textures.cave_hugo_spring_count - 1;
        if (textures.cave_hugo_spring[frame]) {
            SDL_RenderCopy(renderer, textures.cave_hugo_spring[frame], NULL, NULL);
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
        scylla_animation = textures.cave_scylla_bird;
        scylla_count = textures.cave_scylla_bird_count;
    } else if (game_ctx.cave_win_type == 1) {
        scylla_animation = textures.cave_scylla_leaves;
        scylla_count = textures.cave_scylla_leaves_count;
    } else if (game_ctx.cave_win_type == 2) {
        scylla_animation = textures.cave_scylla_ropes;
        scylla_count = textures.cave_scylla_ropes_count;
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
    
    if (textures.cave_scylla_spring && textures.cave_scylla_spring_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_scylla_spring_count) frame = textures.cave_scylla_spring_count - 1;
        if (textures.cave_scylla_spring[frame]) {
            SDL_RenderCopy(renderer, textures.cave_scylla_spring[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_family_cage_opens() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.cave_family_cage && textures.cave_family_cage_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_family_cage_count) frame = textures.cave_family_cage_count - 1;
        if (textures.cave_family_cage[frame]) {
            SDL_RenderCopy(renderer, textures.cave_family_cage[frame], NULL, NULL);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_family_happy() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (textures.cave_happy && textures.cave_happy_count > 0) {
        int frame = get_frame_index();
        if (frame >= textures.cave_happy_count) frame = textures.cave_happy_count - 1;
        if (textures.cave_happy[frame]) {
            SDL_RenderCopy(renderer, textures.cave_happy[frame], NULL, NULL);
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

// Process instructions state
bool process_instructions(InputState state) {
    if (state.key_start) {
        change_state(STATE_FOREST_WAIT_INTRO);
        return true;
    }
    return false;
}

// Process wait intro state
void process_wait_intro(InputState state) {
    // Play intro speech once at start
    static bool intro_played = false;
    if (!intro_played && audio.speak_start) {
        Mix_PlayChannel(-1, audio.speak_start, 0);
        intro_played = true;
    }
    
    if (get_state_time() > 2.0) {
        intro_played = false;  // Reset for next time
        change_state(STATE_FOREST_PLAYING);
    }
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
void process_playing(InputState state) {
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
        change_state(STATE_FOREST_WIN_TALKING);
        return;
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
                change_state(STATE_FOREST_FLYING_START);
                return;
            } else if (obs == OBS_TRAP && !game_ctx.arrow_up_focus) {
                // Trap hit
                if (audio.sfx_hugo_hittrap) Mix_PlayChannel(-1, audio.sfx_hugo_hittrap, 0);
                game_ctx.obstacles[current_pos] = OBS_NONE;
                change_state(STATE_FOREST_TRAP_ANIMATION);
                return;
            } else if (obs == OBS_ROCK && !game_ctx.arrow_up_focus) {
                // Rock hit
                if (audio.sfx_hugo_hitlog) Mix_PlayChannel(-1, audio.sfx_hugo_hitlog, 0);
                game_ctx.obstacles[current_pos] = OBS_NONE;
                change_state(STATE_FOREST_ROCK_ANIMATION);
                return;
            } else if (obs == OBS_TREE && !game_ctx.arrow_down_focus) {
                // Branch hit (need to duck to avoid)
                if (audio.sfx_hugo_hitlog) Mix_PlayChannel(-1, audio.sfx_hugo_hitlog, 0);
                game_ctx.obstacles[current_pos] = OBS_NONE;
                change_state(STATE_FOREST_BRANCH_ANIMATION);
                return;
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
}

// Helper function to reduce lives and transition appropriately
void reduce_lives_and_transition() {
    game_ctx.lives--;
    printf("Hit obstacle! Lives remaining: %d\n", game_ctx.lives);
    
    if (game_ctx.lives <= 0) {
        change_state(STATE_FOREST_TALKING_GAME_OVER);
    } else {
        change_state(STATE_FOREST_TALKING_AFTER_HURT);
    }
}

// Forest hurt state processing functions

void process_forest_branch_animation(InputState state) {
    // Play bird sounds once at start
    static bool audio_played = false;
    if (!audio_played && audio.sfx_birds) {
        Mix_PlayChannel(-1, audio.sfx_birds, 0);
        audio_played = true;
    }
    
    // Branch hit animation - placeholder timing, ideally based on animation frames
    if (get_state_time() > 1.0) {
        audio_played = false;
        change_state(STATE_FOREST_BRANCH_TALKING);
    }
}

void process_forest_branch_talking(InputState state) {
    // Play speech once at start
    static bool audio_played = false;
    if (!audio_played && audio.speak_hitlog) {
        Mix_PlayChannel(-1, audio.speak_hitlog, 0);
        audio_played = true;
    }
    
    // Branch talking - placeholder timing
    if (get_state_time() > 2.0) {
        audio_played = false;
        reduce_lives_and_transition();
    }
}

void process_forest_flying_start(InputState state) {
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
        change_state(STATE_FOREST_FLYING_TALKING);
    }
}

void process_forest_flying_talking(InputState state) {
    // Play speech once at start
    static bool audio_played = false;
    if (!audio_played && audio.speak_catapult_talktop) {
        Mix_PlayChannel(-1, audio.speak_catapult_talktop, 0);
        audio_played = true;
    }
    
    // Hugo talking while at top of flight
    if (get_state_time() > 1.5) {
        audio_played = false;
        change_state(STATE_FOREST_FLYING_FALLING);
    }
}

void process_forest_flying_falling(InputState state) {
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
        change_state(STATE_FOREST_FLYING_FALLING_HANG_ANIMATION);
    }
}

void process_forest_flying_falling_hang_animation(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played && audio.sfx_hugo_hangstart) {
        Mix_PlayChannel(-1, audio.sfx_hugo_hangstart, 0);
        audio_played = true;
    }
    
    // Hugo catching branch animation
    if (get_state_time() > 1.5) {
        audio_played = false;
        change_state(STATE_FOREST_FLYING_FALLING_HANG_TALKING);
    }
}

void process_forest_flying_falling_hang_talking(InputState state) {
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
        reduce_lives_and_transition();
    }
}

void process_forest_rock_animation(InputState state) {
    // Hugo looking at rock
    if (get_state_time() > 0.5) {
        change_state(STATE_FOREST_ROCK_HIT_ANIMATION);
    }
}

void process_forest_rock_hit_animation(InputState state) {
    // Rock hits Hugo
    if (get_state_time() > 1.0) {
        change_state(STATE_FOREST_ROCK_TALKING);
    }
}

void process_forest_rock_talking(InputState state) {
    // Play speech once at start
    static bool audio_played = false;
    if (!audio_played && audio.speak_rock) {
        Mix_PlayChannel(-1, audio.speak_rock, 0);
        audio_played = true;
    }
    
    // Hugo talks after rock hit
    if (get_state_time() > 2.0) {
        audio_played = false;
        reduce_lives_and_transition();
    }
}

void process_forest_trap_animation(InputState state) {
    // Hugo falling into trap
    if (get_state_time() > 1.5) {
        change_state(STATE_FOREST_TRAP_TALKING);
    }
}

void process_forest_trap_talking(InputState state) {
    // Play speech once at start
    static bool audio_played = false;
    if (!audio_played && audio.speak_trap) {
        Mix_PlayChannel(-1, audio.speak_trap, 0);
        audio_played = true;
    }
    
    // Hugo talking after trap
    if (get_state_time() > 2.0) {
        audio_played = false;
        reduce_lives_and_transition();
    }
}

void process_forest_scylla_button(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played && audio.sfx_lightning_warning) {
        Mix_PlayChannel(-1, audio.sfx_lightning_warning, 0);
        audio_played = true;
    }
    
    // Scylla button flashing (not normally triggered in single player)
    if (get_state_time() > 2.0) {
        audio_played = false;
        change_state(STATE_FOREST_PLAYING);
    }
}

void process_forest_talking_after_hurt(InputState state) {
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
        change_state(STATE_FOREST_PLAYING);
    }
}

void process_forest_talking_game_over(InputState state) {
    // Play speech once at start
    static bool audio_played = false;
    if (!audio_played && audio.speak_gameover) {
        Mix_PlayChannel(-1, audio.speak_gameover, 0);
        audio_played = true;
    }
    
    if (get_state_time() > 4.0) {
        audio_played = false;
        change_state(STATE_CAVE_WAITING_BEFORE_TALKING);
    }
}

void process_forest_win_talking(InputState state) {
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
        change_state(STATE_CAVE_WAITING_BEFORE_TALKING);
    }
}

// Process win state (legacy - redirects to win_talking)
void process_win(InputState state) {
    if (get_state_time() > 3.0) {
        // Transition to cave bonus game
        change_state(STATE_CAVE_WAITING_BEFORE_TALKING);
    }
}

// Process game over state
void process_game_over(InputState state) {
    if (get_state_time() > 5.0) {
        change_state(STATE_END);
    }
}

// Cave game processing functions
void process_cave_waiting_before_talking(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played) {
        if (audio.cave_her_er_vi) Mix_PlayChannel(-1, audio.cave_her_er_vi, 0);
        if (audio.cave_stemning) Mix_PlayChannel(-1, audio.cave_stemning, 0);
        audio_played = true;
    }
    
    if (get_state_time() > 2.5) {
        audio_played = false;
        change_state(STATE_CAVE_TALKING_BEFORE_CLIMB);
    }
}

void process_cave_talking_before_climb(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played && audio.cave_trappe_snak) {
        Mix_PlayChannel(-1, audio.cave_trappe_snak, 0);
        audio_played = true;
    }
    
    // Hugo talks before climbing (~4 seconds based on sync timing)
    if (get_state_time() > 4.0) {
        audio_played = false;
        change_state(STATE_CAVE_CLIMBING);
    }
}

void process_cave_intro(InputState state) {
    if (get_state_time() > 2.5) {
        change_state(STATE_CAVE_CLIMBING);
    }
}

void process_cave_climbing(InputState state) {
    if (get_state_time() > 5.1) {
        change_state(STATE_CAVE_WAITING_INPUT);
    }
}

bool process_cave_waiting_input(InputState state) {
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
            change_state(STATE_CAVE_GOING_ROPE);
            return true;
        } else if (event->key.keysym.sym == SDLK_6) {
            if (audio.cave_tast_trykket) Mix_PlayChannel(-1, audio.cave_tast_trykket, 0);
            game_ctx.cave_selected_rope = 1;
            audio_played = false;
            change_state(STATE_CAVE_GOING_ROPE);
            return true;
        } else if (event->key.keysym.sym == SDLK_9) {
            if (audio.cave_tast_trykket) Mix_PlayChannel(-1, audio.cave_tast_trykket, 0);
            game_ctx.cave_selected_rope = 2;
            audio_played = false;
            change_state(STATE_CAVE_GOING_ROPE);
            return true;
        }
    }*/
    return false;
}

void process_cave_going_rope(InputState state) {
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
            change_state(STATE_CAVE_LOST);
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
            change_state(STATE_CAVE_SCYLLA_LOST);
        }
    }
}

void process_cave_lost(InputState state) {
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
        change_state(STATE_CAVE_LOST_SPRING);
    }
}

void process_cave_lost_spring(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played && audio.cave_fjeder) {
        Mix_PlayChannel(-1, audio.cave_fjeder, 0);
        audio_played = true;
    }
    
    if (get_state_time() > 3.9) {
        audio_played = false;
        change_state(STATE_END);
    }
}

void process_cave_scylla_lost(InputState state) {
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
            change_state(STATE_CAVE_SCYLLA_SPRING);
        } else {
            change_state(STATE_CAVE_FAMILY_CAGE_OPENS);
        }
    }
}

void process_cave_scylla_spring(InputState state) {
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
        change_state(STATE_CAVE_FAMILY_CAGE_OPENS);
    }
}

void process_cave_family_cage_opens(InputState state) {
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
        change_state(STATE_CAVE_FAMILY_HAPPY);
    }
}

void process_cave_family_happy(InputState state) {
    // Play audio once at start
    static bool audio_played = false;
    if (!audio_played && audio.cave_fanfare) {
        Mix_PlayChannel(-1, audio.cave_fanfare, 0);
        audio_played = true;
    }
    
    if (get_state_time() > 5.0) {
        audio_played = false;
        change_state(STATE_END);
    }
}

// Main game loop
void game_loop() {
    bool quit = false;
    SDL_Event event;
    
    // Initialize
    init_game_context();
    change_state(STATE_INSTRUCTIONS);
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
        
        // Process and render current state
        switch (state_info.current_state) {
            case STATE_INSTRUCTIONS:
                process_instructions(input_state);
                render_instructions();
                break;
                
            // Forest states
            case STATE_FOREST_WAIT_INTRO:
                process_wait_intro(input_state);
                render_wait_intro();
                break;
            case STATE_FOREST_PLAYING:
                process_playing(input_state);
                render_playing();
                break;
            case STATE_FOREST_BRANCH_ANIMATION:
                process_forest_branch_animation(input_state);
                render_forest_branch_animation();
                break;
            case STATE_FOREST_BRANCH_TALKING:
                process_forest_branch_talking(input_state);
                render_forest_branch_talking();
                break;
            case STATE_FOREST_FLYING_START:
                process_forest_flying_start(input_state);
                render_forest_flying_start();
                break;
            case STATE_FOREST_FLYING_TALKING:
                process_forest_flying_talking(input_state);
                render_forest_flying_talking();
                break;
            case STATE_FOREST_FLYING_FALLING:
                process_forest_flying_falling(input_state);
                render_forest_flying_falling();
                break;
            case STATE_FOREST_FLYING_FALLING_HANG_ANIMATION:
                process_forest_flying_falling_hang_animation(input_state);
                render_forest_flying_falling_hang_animation();
                break;
            case STATE_FOREST_FLYING_FALLING_HANG_TALKING:
                process_forest_flying_falling_hang_talking(input_state);
                render_forest_flying_falling_hang_talking();
                break;
            case STATE_FOREST_ROCK_ANIMATION:
                process_forest_rock_animation(input_state);
                render_forest_rock_animation();
                break;
            case STATE_FOREST_ROCK_HIT_ANIMATION:
                process_forest_rock_hit_animation(input_state);
                render_forest_rock_hit_animation();
                break;
            case STATE_FOREST_ROCK_TALKING:
                process_forest_rock_talking(input_state);
                render_forest_rock_talking();
                break;
            case STATE_FOREST_TRAP_ANIMATION:
                process_forest_trap_animation(input_state);
                render_forest_trap_animation();
                break;
            case STATE_FOREST_TRAP_TALKING:
                process_forest_trap_talking(input_state);
                render_forest_trap_talking();
                break;
            case STATE_FOREST_SCYLLA_BUTTON:
                process_forest_scylla_button(input_state);
                render_forest_scylla_button();
                break;
            case STATE_FOREST_TALKING_AFTER_HURT:
                process_forest_talking_after_hurt(input_state);
                render_forest_talking_after_hurt();
                break;
            case STATE_FOREST_TALKING_GAME_OVER:
                process_forest_talking_game_over(input_state);
                render_forest_talking_game_over();
                break;
            case STATE_FOREST_WIN_TALKING:
                process_forest_win_talking(input_state);
                render_forest_win_talking();
                break;
                
            // Cave states
            case STATE_CAVE_WAITING_BEFORE_TALKING:
                process_cave_waiting_before_talking(input_state);
                render_cave_waiting_before_talking();
                break;
            case STATE_CAVE_TALKING_BEFORE_CLIMB:
                process_cave_talking_before_climb(input_state);
                render_cave_talking_before_climb();
                break;
            case STATE_CAVE_CLIMBING:
                process_cave_climbing(input_state);
                render_cave_climbing();
                break;
            case STATE_CAVE_WAITING_INPUT:
                process_cave_waiting_input(input_state);
                render_cave_waiting_input();
                break;
            case STATE_CAVE_GOING_ROPE:
                process_cave_going_rope(input_state);
                render_cave_going_rope();
                break;
            case STATE_CAVE_LOST:
                process_cave_lost(input_state);
                render_cave_lost();
                break;
            case STATE_CAVE_LOST_SPRING:
                process_cave_lost_spring(input_state);
                render_cave_lost_spring();
                break;
            case STATE_CAVE_SCYLLA_LOST:
                process_cave_scylla_lost(input_state);
                render_cave_scylla_lost();
                break;
            case STATE_CAVE_SCYLLA_SPRING:
                process_cave_scylla_spring(input_state);
                render_cave_scylla_spring();
                break;
            case STATE_CAVE_FAMILY_CAGE_OPENS:
                process_cave_family_cage_opens(input_state);
                render_cave_family_cage_opens();
                break;
            case STATE_CAVE_FAMILY_HAPPY:
                process_cave_family_happy(input_state);
                render_cave_family_happy();
                break;
                
            case STATE_END:
                quit = true;
                break;
        }
        
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
