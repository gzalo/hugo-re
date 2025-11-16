#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

#ifndef COMMON_H
#define COMMON_H

// Game constants
#define FOREST_MAX_TIME 60
#define FOREST_GROUND_SPEED 75
#define HUGO_X_POS 16
#define START_LIVES 3
#define INSTRUCTIONS_TIMEOUT 3

// Screen dimensions
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define WINDOW_SCALE 2

typedef enum {
    STATE_NONE,
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

typedef enum {
    RENDER_PRE,
    RENDER_POST,
} RenderType;

typedef struct {
    int score;
    int lives;
    double parallax_pos;
    ObstacleType obstacles[FOREST_MAX_TIME];
    int sacks[FOREST_MAX_TIME];
    int leaves[FOREST_MAX_TIME];
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
    SDL_Texture** frames;
    int frame_count;
    int* sync_data;
    int sync_count;
} Animation;

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
    Animation hugohitlog;          // branch hit animation (0-42)
    Animation hugohitlog_talk;     // branch talking (0-17)
    Animation catapult_fly;        // flying up (0-113)
    Animation catapult_fall;       // falling down (0-74)
    Animation catapult_airtalk;    // talking in air (0-15)
    Animation catapult_hang;       // hanging (0-12)
    Animation catapult_hangspeak;  // hanging mouth (0-11)
    Animation hugo_lookrock;       // looking at rock (0-14)
    Animation hit_rock;            // hit by rock (0-60)
    Animation hit_rock_sync;       // rock talking (0-17)
    Animation hugo_traphurt;       // trap hurt (0-9)
    Animation hugo_traptalk;       // trap talking (0-15)
    
    // Cave animations
    Animation cave_talks;          // cave intro talking (0-12)
    Animation cave_climbs;         // climbing (0-40)
    Animation cave_first_rope;     // first rope descent (0-32)
    Animation cave_second_rope;    // second rope descent (0-39)
    Animation cave_third_rope;     // third rope descent (0-48)
    Animation cave_scylla_bird;    // scylla bird (0-62)
    Animation cave_scylla_leaves;  // scylla leaves (0-55)
    Animation cave_scylla_ropes;   // scylla ropes (0-42)
    Animation cave_scylla_spring;  // scylla spring (0-34)
    Animation cave_hugo_puff_first;  // hugo puff first rope (0-44)
    Animation cave_hugo_puff_second; // hugo puff second rope (0-44)
    Animation cave_hugo_puff_third;  // hugo puff third rope (0-44)
    Animation cave_hugo_spring;    // hugo spring (0-38)
    Animation cave_family_cage;    // family cage opens (0-33)
    Animation cave_happy;          // happy ending (0-111)
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

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern GameContext game_ctx;
extern StateInfo state_info;
extern GameTextures textures;
extern GameAudio audio;

typedef GameState (*ProcessFunc)(InputState state);
typedef void (*RenderFunc)(void);

#endif