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

// SDL-related typedefs
typedef SDL_Texture Texture;
typedef Mix_Chunk Audio;

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
    Texture** frames;
    int frame_count;
    int* sync_data;
    int sync_count;
} Animation;

Texture* animation_get_sync_frame(Animation animation, int frame_index);
Texture* animation_get_frame(Animation animation, int frame_index);

typedef struct {
    // Fixed assets
    Texture* instruction_screen;
    Texture* arrows[4];  // up, up_pressed, down, down_pressed
    Texture* bg_gradient;
    
    // Background layers
    Texture* bg_hillsday;
    Texture* bg_trees;
    Texture* bg_ground;
    Texture* grass;
    Texture* leaves1;
    Texture* leaves2;
    Texture* end_mountain;
    
    // Hugo animations
    Texture* hugo_side[8];     // walking (0-7)
    Texture* hugo_jump[3];     // jumping (0-2)
    Texture* hugo_crawl[8];    // crawling (0-7)
    Texture* hugo_telllives[16]; // intro animation (0-15)
    
    // Obstacles
    Texture* catapult[8];      // catapult (0-7)
    Texture* trap[6];          // trap (0-5)
    Texture* rock[8];          // rolling rock (0-7)
    Texture* tree[7];          // branch swinging (0-6)
    Texture* lone_tree;        // tree trunk
    Texture* sack[4];          // sacks (0-3)
    
    // HUD elements
    Texture* scoreboard;
    Texture* score_numbers;    // score digits spritesheet
    Texture* hugo_lives;       // life indicator
    
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
    Texture* cave_hugo_sprite;     // hugo standing sprite
} GameTextures;

typedef struct {
    // Forest speech
    Audio* speak_start;
    Audio* speak_rock;
    Audio* speak_dieonce;
    Audio* speak_trap;
    Audio* speak_lastlife;
    Audio* speak_catapult_up;
    Audio* speak_catapult_hit;
    Audio* speak_catapult_talktop;
    Audio* speak_catapult_down;
    Audio* speak_catapult_hang;
    Audio* speak_hitlog;
    Audio* speak_gameover;
    Audio* speak_levelcompleted;
    
    // Forest sound effects
    Audio* sfx_bg_atmosphere;
    Audio* sfx_lightning_warning;
    Audio* sfx_hugo_knock;
    Audio* sfx_hugo_hittrap;
    Audio* sfx_hugo_launch;
    Audio* sfx_sack_normal;
    Audio* sfx_sack_bonus;
    Audio* sfx_tree_swush;
    Audio* sfx_hugo_hitlog;
    Audio* sfx_catapult_eject;
    Audio* sfx_birds;
    Audio* sfx_hugo_hitscreen;
    Audio* sfx_hugo_screenklir;
    Audio* sfx_hugo_crash;
    Audio* sfx_hugo_hangstart;
    Audio* sfx_hugo_hang;
    Audio* sfx_hugo_walk[5];
    
    // Cave speech
    Audio* cave_her_er_vi;
    Audio* cave_trappe_snak;
    Audio* cave_nu_kommer_jeg;
    Audio* cave_afskylia_snak;
    Audio* cave_hugo_katapult;
    Audio* cave_hugo_skyd_ud;
    Audio* cave_afskylia_skyd_ud;
    Audio* cave_hugoline_tak;
    
    // Cave sound effects
    Audio* cave_stemning;
    Audio* cave_fodtrin1;
    Audio* cave_fodtrin2;
    Audio* cave_hiv_i_reb;
    Audio* cave_fjeder;
    Audio* cave_pre_puf;
    Audio* cave_puf;
    Audio* cave_tast_trykket;
    Audio* cave_pre_fanfare;
    Audio* cave_fanfare;
    Audio* cave_fugle_skrig;
    Audio* cave_trappe_grin;
    Audio* cave_skrig;
    Audio* cave_score_counter;
} GameAudio;

typedef struct {
    bool key_up;
    bool key_down;
    bool key_start;
    bool cave_rope_1_pressed;
    bool cave_rope_2_pressed;
    bool cave_rope_3_pressed;
} InputState;

extern GameContext game_ctx;
extern StateInfo state_info;
extern GameTextures textures;
extern GameAudio audio;

typedef GameState (*ProcessFunc)(InputState state);
typedef void (*RenderFunc)(void);
typedef void (*OnEnterFunc)(void);

void render(Texture *texture, int x, int y);
void play(Audio *audio);

#endif