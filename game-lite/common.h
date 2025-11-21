#include "render_sdl.h"
#include <stdbool.h>
#include "config.h"
#include "resources.h"

#ifndef COMMON_H
#define COMMON_H

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
    Animation hugo_telllives; // intro animation (0-15)
    Texture* hugo_hand1;    // hand frame 1
    Texture* hugo_hand2;    // hand frame 2
    
    // Sync data for talking animations (loaded from .oos.json files)
    int* sync_start;        // sync for speak_start (005-01)
    int sync_start_count;
    int* sync_rock;         // sync for speak_rock (005-02)
    int sync_rock_count;
    int* sync_dieonce;      // sync for speak_dieonce (005-03)
    int sync_dieonce_count;
    int* sync_trap;         // sync for speak_trap (005-04)
    int sync_trap_count;
    int* sync_lastlife;     // sync for speak_lastlife (005-05)
    int sync_lastlife_count;
    int* sync_catapult_talktop;  // sync for speak_catapult_talktop (005-08)
    int sync_catapult_talktop_count;
    int* sync_catapult_hang;     // sync for speak_catapult_hang (005-10)
    int sync_catapult_hang_count;
    int* sync_hitlog;       // sync for speak_hitlog (005-11)
    int sync_hitlog_count;
    int* sync_gameover;     // sync for speak_gameover (005-12)
    int sync_gameover_count;
    int* sync_levelcompleted;    // sync for speak_levelcompleted (005-13)
    int sync_levelcompleted_count;
    
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
    Texture* cave_score_font[10];  // cave score digits (0-9)
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

typedef enum {
    RENDER_NONE,
    RENDER_PRE,
    RENDER_POST
} RenderStage;

extern GameTextures textures;
extern GameAudio audio;

typedef enum {
    STATE_NONE,
    STATE_INSTRUCTIONS,
    STATE_FOREST,
    STATE_CAVE,
    STATE_END
} GameState;

#endif