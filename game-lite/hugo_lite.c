/*
 * Hugo Lite - Single Player C/SDL2 Version
 * A simplified version of the Hugo forest game (the main game)
 * Based on the Python implementation in ../game/forest/
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
#define MAX_OBSTACLES 60
#define MAX_LIVES 3

// Game states
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

// Obstacle types
typedef enum {
    OBS_NONE = 0,
    OBS_CATAPULT = 1,
    OBS_TRAP = 2,
    OBS_ROCK = 3,
    OBS_TREE = 4
} ObstacleType;

// Game context
typedef struct {
    int score;
    int lives;
    double parallax_pos;
    ObstacleType obstacles[MAX_OBSTACLES];
    int sacks[MAX_OBSTACLES];
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

// State timing structure
typedef struct {
    double start_time;
    GameState current_state;
    int frame_count;
} StateInfo;

// Texture resources (matching forest_resources.py)
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
} GameTextures;

// Global game variables
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameContext game_ctx;
StateInfo state_info;
GameTextures textures;

// Animation frame tracking
double get_state_time() {
    return (SDL_GetTicks() / 1000.0) - state_info.start_time;
}

int get_frame_index() {
    return (int)(get_state_time() * 10);
}

double get_game_time() {
    return SDL_GetTicks() / 1000.0;
}

// Load a single texture (returns NULL if file doesn't exist)
SDL_Texture* load_texture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        return NULL;  // File doesn't exist, will use fallback rendering
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        printf("Warning: Could not create texture from %s: %s\n", path, SDL_GetError());
    }
    
    return texture;
}

// Load animation frames
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
    return loaded;
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
    
    printf("Loaded %d textures (data_dir: %s)\n", loaded_count, data_dir);
}

// Free all textures
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
}

// Initialize SDL
bool init_sdl(const char *data_dir) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Hugo Lite - Forest Game",
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

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    // Initialize SDL_mixer (optional)
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Warning: SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        printf("Continuing without audio...\n");
    } else {
        Mix_AllocateChannels(16);
    }
    
    // Load game textures
    init_textures(data_dir);

    return true;
}

// Generate random obstacles
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

// Generate sacks
void generate_sacks() {
    for (int i = 0; i < FOREST_MAX_TIME; i++) {
        int r = rand() % 100;
        if (r < 70) {
            game_ctx.sacks[i] = 0; // No sack
        } else if (r < 91) {
            game_ctx.sacks[i] = 1; // Normal sack (100 pts)
        } else {
            game_ctx.sacks[i] = 2; // Bonus sack (250 pts)
        }
    }
}

// Initialize game context
void init_game_context() {
    game_ctx.score = 0;
    game_ctx.lives = MAX_LIVES;
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

// State transition
void change_state(GameState new_state) {
    printf("State transition: %d -> %d\n", state_info.current_state, new_state);
    state_info.current_state = new_state;
    state_info.start_time = SDL_GetTicks() / 1000.0;
    state_info.frame_count = 0;
}

// Draw simple text (using rectangles to represent text)
void draw_text_box(int x, int y, int w, int h, const char* text) {
    SDL_Rect box = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &box);
    
    // In a full implementation, would render actual text
    // For now, just show a box where text would go
    (void)text; // Suppress unused parameter warning
}

// Render instructions screen
void render_instructions() {
    SDL_SetRenderDrawColor(renderer, 20, 50, 100, 255);
    SDL_RenderClear(renderer);
    
    // Use the actual instruction image if available
    if (textures.instruction_screen) {
        SDL_RenderCopy(renderer, textures.instruction_screen, NULL, NULL);
    } else {
        // Fallback to text boxes
        draw_text_box(40, 20, 240, 30, "HUGO - FOREST GAME");
        draw_text_box(30, 60, 260, 20, "Press UP (2) to JUMP");
        draw_text_box(30, 90, 260, 20, "Press DOWN (8) to DUCK");
        draw_text_box(30, 120, 260, 20, "Collect sacks for points!");
        draw_text_box(30, 150, 260, 20, "Avoid obstacles!");
    }
    
    // Start prompt (always show blinking text)
    if (get_frame_index() % 10 < 5) {
        draw_text_box(70, 190, 180, 25, "Press 5 to START");
    }
    
    SDL_RenderPresent(renderer);
}

// Render intro animation
void render_wait_intro() {
    SDL_SetRenderDrawColor(renderer, 50, 100, 50, 255);
    SDL_RenderClear(renderer);
    
    // Show Hugo intro animation
    if (textures.hugo_telllives[0]) {
        int frame = get_frame_index() % 16;
        SDL_Rect hugo = {128, -16, 64, 256};
        SDL_RenderCopy(renderer, textures.hugo_telllives[frame], NULL, &hugo);
    } else {
        // Fallback rectangle
        SDL_Rect hugo = {130, 100, 60, 80};
        SDL_SetRenderDrawColor(renderer, 255, 200, 100, 255);
        SDL_RenderFillRect(renderer, &hugo);
    }
    
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
    if (textures.bg_ground) {
        int ground_offset = (int)(game_ctx.parallax_pos * FOREST_GROUND_SPEED) % 320;
        for (int i = 0; i < 5; i++) {
            SDL_Rect ground_rect = {i * 320 - ground_offset, 158, 320, 82};
            SDL_RenderCopy(renderer, textures.bg_ground, NULL, &ground_rect);
        }
    } else {
        // Fallback solid ground
        SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
        SDL_Rect ground = {0, 158, SCREEN_WIDTH, SCREEN_HEIGHT - 158};
        SDL_RenderFillRect(renderer, &ground);
    }
    
    // Render grass layer
    if (textures.grass) {
        int grass_offset = (int)(game_ctx.parallax_pos * 30) % 320;
        for (int i = 0; i < 12; i++) {
            SDL_Rect grass_rect = {i * 27 - grass_offset, 172, 27, 68};
            SDL_RenderCopy(renderer, textures.grass, NULL, &grass_rect);
        }
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
    if (textures.score_numbers) {
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
    } else {
        // Fallback: simple colored blocks
        int score_display = game_ctx.score;
        for (int i = 0; i < 4; i++) {
            score_display /= 10;
            SDL_Rect digit_box = {200 + (3-i) * 24, 194, 20, 28};
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &digit_box);
        }
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
        if (textures.sack[0]) {
            int sack_frame = frame % 4;
            SDL_RenderCopy(renderer, textures.sack[sack_frame], NULL, &sack);
        } else {
            // Fallback colors
            if (game_ctx.sacks[i] == 1) {
                SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255); // Normal sack
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Bonus sack
            }
            SDL_RenderFillRect(renderer, &sack);
        }
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
    if (textures.arrows[0] && textures.arrows[1] && textures.arrows[2] && textures.arrows[3]) {
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
    } else {
        // Fallback to colored rectangles
        if (game_ctx.arrow_up_focus) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        }
        SDL_RenderFillRect(renderer, &up_arrow);
        
        if (game_ctx.arrow_down_focus) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        }
        SDL_RenderFillRect(renderer, &down_arrow);
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

// Render win screen
void render_win() {
    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(80, 60, 160, 40, "YOU WIN!");
    
    char score_text[50];
    snprintf(score_text, sizeof(score_text), "Score: %d", game_ctx.score);
    draw_text_box(80, 110, 160, 30, score_text);
    
    draw_text_box(50, 160, 220, 30, "Cave bonus coming!");
    
    SDL_RenderPresent(renderer);
}

// Render game over screen
void render_game_over() {
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(60, 80, 200, 40, "GAME OVER");
    
    char score_text[50];
    snprintf(score_text, sizeof(score_text), "Score: %d", game_ctx.score);
    draw_text_box(80, 130, 160, 30, score_text);
    
    SDL_RenderPresent(renderer);
}

// Forest hurt state rendering functions
void render_forest_branch_animation() {
    SDL_SetRenderDrawColor(renderer, 100, 100, 200, 255);
    SDL_RenderClear(renderer);
    draw_text_box(70, 100, 180, 40, "Branch hit!");
    SDL_RenderPresent(renderer);
}

void render_forest_branch_talking() {
    SDL_SetRenderDrawColor(renderer, 120, 120, 220, 255);
    SDL_RenderClear(renderer);
    draw_text_box(60, 100, 200, 40, "Hugo: Ouch!");
    SDL_RenderPresent(renderer);
}

void render_forest_flying_start() {
    SDL_SetRenderDrawColor(renderer, 150, 150, 255, 255);
    SDL_RenderClear(renderer);
    draw_text_box(60, 100, 200, 40, "Hugo flies up!");
    SDL_RenderPresent(renderer);
}

void render_forest_flying_talking() {
    SDL_SetRenderDrawColor(renderer, 170, 170, 255, 255);
    SDL_RenderClear(renderer);
    draw_text_box(50, 100, 220, 40, "Hugo: Whoa!");
    SDL_RenderPresent(renderer);
}

void render_forest_flying_falling() {
    SDL_SetRenderDrawColor(renderer, 190, 150, 255, 255);
    SDL_RenderClear(renderer);
    draw_text_box(60, 100, 200, 40, "Hugo falling...");
    SDL_RenderPresent(renderer);
}

void render_forest_flying_falling_hang_animation() {
    SDL_SetRenderDrawColor(renderer, 200, 170, 255, 255);
    SDL_RenderClear(renderer);
    draw_text_box(50, 100, 220, 40, "Catching branch!");
    SDL_RenderPresent(renderer);
}

void render_forest_flying_falling_hang_talking() {
    SDL_SetRenderDrawColor(renderer, 220, 190, 255, 255);
    SDL_RenderClear(renderer);
    draw_text_box(50, 100, 220, 40, "Hugo: Got it!");
    SDL_RenderPresent(renderer);
}

void render_forest_rock_animation() {
    SDL_SetRenderDrawColor(renderer, 150, 100, 100, 255);
    SDL_RenderClear(renderer);
    draw_text_box(60, 100, 200, 40, "Rock incoming!");
    SDL_RenderPresent(renderer);
}

void render_forest_rock_hit_animation() {
    SDL_SetRenderDrawColor(renderer, 180, 100, 100, 255);
    SDL_RenderClear(renderer);
    draw_text_box(70, 100, 180, 40, "Rock hits!");
    SDL_RenderPresent(renderer);
}

void render_forest_rock_talking() {
    SDL_SetRenderDrawColor(renderer, 200, 120, 120, 255);
    SDL_RenderClear(renderer);
    draw_text_box(60, 100, 200, 40, "Hugo: Ow!");
    SDL_RenderPresent(renderer);
}

void render_forest_trap_animation() {
    SDL_SetRenderDrawColor(renderer, 120, 120, 80, 255);
    SDL_RenderClear(renderer);
    draw_text_box(60, 100, 200, 40, "Fell in trap!");
    SDL_RenderPresent(renderer);
}

void render_forest_trap_talking() {
    SDL_SetRenderDrawColor(renderer, 140, 140, 100, 255);
    SDL_RenderClear(renderer);
    draw_text_box(60, 100, 200, 40, "Hugo: Help!");
    SDL_RenderPresent(renderer);
}

void render_forest_scylla_button() {
    SDL_SetRenderDrawColor(renderer, 200, 0, 200, 255);
    SDL_RenderClear(renderer);
    
    // Flashing Scylla button
    if (get_frame_index() % 2 == 0) {
        draw_text_box(60, 100, 200, 40, "SCYLLA!");
    }
    
    SDL_RenderPresent(renderer);
}

void render_forest_talking_after_hurt() {
    SDL_SetRenderDrawColor(renderer, 100, 150, 100, 255);
    SDL_RenderClear(renderer);
    
    char lives_text[50];
    if (game_ctx.lives == 1) {
        snprintf(lives_text, sizeof(lives_text), "Last life!");
    } else {
        snprintf(lives_text, sizeof(lives_text), "%d lives left", game_ctx.lives);
    }
    
    draw_text_box(60, 80, 200, 40, "Hugo hurt!");
    draw_text_box(70, 130, 180, 30, lives_text);
    
    SDL_RenderPresent(renderer);
}

void render_forest_talking_game_over() {
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(60, 80, 200, 40, "GAME OVER");
    
    char score_text[50];
    snprintf(score_text, sizeof(score_text), "Score: %d", game_ctx.score);
    draw_text_box(80, 130, 160, 30, score_text);
    
    SDL_RenderPresent(renderer);
}

void render_forest_win_talking() {
    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(60, 60, 200, 40, "Forest cleared!");
    
    char score_text[50];
    snprintf(score_text, sizeof(score_text), "Score: %d", game_ctx.score);
    draw_text_box(80, 110, 160, 30, score_text);
    
    draw_text_box(50, 160, 220, 30, "Cave bonus coming!");
    
    SDL_RenderPresent(renderer);
}

// Cave game rendering functions
void render_cave_waiting_before_talking() {
    SDL_SetRenderDrawColor(renderer, 40, 40, 90, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(60, 100, 200, 40, "Entering cave...");
    
    SDL_RenderPresent(renderer);
}

void render_cave_talking_before_climb() {
    SDL_SetRenderDrawColor(renderer, 50, 50, 100, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(50, 100, 220, 40, "Hugo: Let's go!");
    
    SDL_RenderPresent(renderer);
}

// Cave game rendering functions
void render_cave_intro() {
    SDL_SetRenderDrawColor(renderer, 50, 50, 100, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(60, 80, 200, 40, "BONUS: CAVE!");
    draw_text_box(50, 130, 220, 30, "Improve your score!");
    
    SDL_RenderPresent(renderer);
}

void render_cave_climbing() {
    SDL_SetRenderDrawColor(renderer, 60, 60, 120, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(80, 100, 160, 40, "Climbing...");
    
    SDL_RenderPresent(renderer);
}

void render_cave_waiting_input() {
    SDL_SetRenderDrawColor(renderer, 70, 70, 140, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(40, 60, 240, 30, "Choose a rope!");
    draw_text_box(80, 100, 160, 25, "Press 3, 6, or 9");
    
    // Draw three rope indicators
    SDL_Rect rope1 = {60, 150, 30, 50};
    SDL_Rect rope2 = {145, 150, 30, 50};
    SDL_Rect rope3 = {230, 150, 30, 50};
    
    SDL_SetRenderDrawColor(renderer, 200, 150, 100, 255);
    SDL_RenderFillRect(renderer, &rope1);
    SDL_RenderFillRect(renderer, &rope2);
    SDL_RenderFillRect(renderer, &rope3);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &rope1);
    SDL_RenderDrawRect(renderer, &rope2);
    SDL_RenderDrawRect(renderer, &rope3);
    
    SDL_RenderPresent(renderer);
}

void render_cave_going_rope() {
    SDL_SetRenderDrawColor(renderer, 80, 80, 160, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(60, 80, 200, 40, "Going down...");
    
    // Show selected rope
    if (game_ctx.cave_selected_rope >= 0) {
        int rope_x[] = {60, 145, 230};
        SDL_Rect selected_rope = {rope_x[game_ctx.cave_selected_rope], 150, 30, 50};
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &selected_rope);
    }
    
    SDL_RenderPresent(renderer);
}

void render_cave_lost() {
    SDL_SetRenderDrawColor(renderer, 150, 50, 50, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(80, 100, 160, 40, "Oh no!");
    
    SDL_RenderPresent(renderer);
}

void render_cave_lost_spring() {
    SDL_SetRenderDrawColor(renderer, 180, 80, 80, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(60, 80, 200, 40, "Hugo bounces!");
    
    char score_text[50];
    snprintf(score_text, sizeof(score_text), "Final: %d", game_ctx.score);
    draw_text_box(80, 140, 160, 30, score_text);
    
    SDL_RenderPresent(renderer);
}

void render_cave_scylla_lost() {
    // Scylla loses (Hugo wins)
    SDL_SetRenderDrawColor(renderer, 50, 150, 50, 255);
    SDL_RenderClear(renderer);
    
    const char* win_types[] = {"Bird!", "Leaves!", "Ropes!"};
    const char* multipliers[] = {"Score x2", "Score x3", "Score x4"};
    
    draw_text_box(70, 80, 180, 40, win_types[game_ctx.cave_win_type]);
    draw_text_box(70, 130, 180, 30, multipliers[game_ctx.cave_win_type]);
    
    SDL_RenderPresent(renderer);
}

void render_cave_scylla_spring() {
    SDL_SetRenderDrawColor(renderer, 80, 180, 80, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(60, 100, 200, 40, "Scylla springs!");
    
    SDL_RenderPresent(renderer);
}

void render_cave_family_cage_opens() {
    SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(60, 100, 200, 40, "Cage opening!");
    
    SDL_RenderPresent(renderer);
}

void render_cave_family_happy() {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderClear(renderer);
    
    draw_text_box(60, 60, 200, 40, "VICTORY!");
    draw_text_box(60, 110, 200, 30, "Family saved!");
    
    char score_text[50];
    snprintf(score_text, sizeof(score_text), "Final: %d", game_ctx.score);
    draw_text_box(80, 160, 160, 30, score_text);
    
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
bool process_instructions(SDL_Event* event) {
    if (event && event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_5) {
            change_state(STATE_FOREST_WAIT_INTRO);
            return true;
        }
    }
    return false;
}

// Process wait intro state
void process_wait_intro() {
    if (get_state_time() > 2.0) {
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
void process_playing(SDL_Event* event) {
    // Handle input
    if (event && event->type == SDL_KEYDOWN) {
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
    }
    
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
                game_ctx.obstacles[current_pos] = OBS_NONE;
                change_state(STATE_FOREST_FLYING_START);
                return;
            } else if (obs == OBS_TRAP && !game_ctx.arrow_up_focus) {
                // Trap hit
                game_ctx.obstacles[current_pos] = OBS_NONE;
                change_state(STATE_FOREST_TRAP_ANIMATION);
                return;
            } else if (obs == OBS_ROCK && !game_ctx.arrow_up_focus) {
                // Rock hit
                game_ctx.obstacles[current_pos] = OBS_NONE;
                change_state(STATE_FOREST_ROCK_ANIMATION);
                return;
            } else if (obs == OBS_TREE && !game_ctx.arrow_down_focus) {
                // Branch hit (need to duck to avoid)
                game_ctx.obstacles[current_pos] = OBS_NONE;
                change_state(STATE_FOREST_BRANCH_ANIMATION);
                return;
            }
        }
        
        // Check sack collection
        if (game_ctx.arrow_up_focus && game_ctx.sacks[current_pos] != 0) {
            if (game_ctx.sacks[current_pos] == 1) {
                game_ctx.score += 100;
            } else {
                game_ctx.score += 250;
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

void process_forest_branch_animation() {
    // Branch hit animation - placeholder timing, ideally based on animation frames
    if (get_state_time() > 1.0) {
        change_state(STATE_FOREST_BRANCH_TALKING);
    }
}

void process_forest_branch_talking() {
    // Branch talking - placeholder timing
    if (get_state_time() > 2.0) {
        reduce_lives_and_transition();
    }
}

void process_forest_flying_start() {
    // Hugo flying up from catapult
    if (get_state_time() > 3.0) {
        change_state(STATE_FOREST_FLYING_TALKING);
    }
}

void process_forest_flying_talking() {
    // Hugo talking while at top of flight
    if (get_state_time() > 1.5) {
        change_state(STATE_FOREST_FLYING_FALLING);
    }
}

void process_forest_flying_falling() {
    // Hugo falling down
    if (get_state_time() > 1.0) {
        change_state(STATE_FOREST_FLYING_FALLING_HANG_ANIMATION);
    }
}

void process_forest_flying_falling_hang_animation() {
    // Hugo catching branch animation
    if (get_state_time() > 1.5) {
        change_state(STATE_FOREST_FLYING_FALLING_HANG_TALKING);
    }
}

void process_forest_flying_falling_hang_talking() {
    // Hugo talking while hanging
    if (get_state_time() > 2.0) {
        reduce_lives_and_transition();
    }
}

void process_forest_rock_animation() {
    // Hugo looking at rock
    if (get_state_time() > 0.5) {
        change_state(STATE_FOREST_ROCK_HIT_ANIMATION);
    }
}

void process_forest_rock_hit_animation() {
    // Rock hits Hugo
    if (get_state_time() > 1.0) {
        change_state(STATE_FOREST_ROCK_TALKING);
    }
}

void process_forest_rock_talking() {
    // Hugo talks after rock hit
    if (get_state_time() > 2.0) {
        reduce_lives_and_transition();
    }
}

void process_forest_trap_animation() {
    // Hugo falling into trap
    if (get_state_time() > 1.5) {
        change_state(STATE_FOREST_TRAP_TALKING);
    }
}

void process_forest_trap_talking() {
    // Hugo talking after trap
    if (get_state_time() > 2.0) {
        reduce_lives_and_transition();
    }
}

void process_forest_scylla_button() {
    // Scylla button flashing (not normally triggered in single player)
    if (get_state_time() > 2.0) {
        change_state(STATE_FOREST_PLAYING);
    }
}

void process_forest_talking_after_hurt() {
    // Hugo talks after getting hurt (still has lives)
    double duration = (game_ctx.lives == 1) ? 3.0 : 2.5;
    if (get_state_time() > duration) {
        change_state(STATE_FOREST_PLAYING);
    }
}

void process_forest_talking_game_over() {
    // Hugo talks about game over
    if (get_state_time() > 4.0) {
        change_state(STATE_END);
    }
}

void process_forest_win_talking() {
    // Hugo talks after winning forest
    if (get_state_time() > 3.0) {
        // Transition to cave bonus game
        change_state(STATE_CAVE_WAITING_BEFORE_TALKING);
    }
}

// Process win state (legacy - redirects to win_talking)
void process_win() {
    if (get_state_time() > 3.0) {
        // Transition to cave bonus game
        change_state(STATE_CAVE_WAITING_BEFORE_TALKING);
    }
}

// Process game over state
void process_game_over() {
    if (get_state_time() > 5.0) {
        change_state(STATE_END);
    }
}

// Cave game processing functions
void process_cave_waiting_before_talking() {
    if (get_state_time() > 2.5) {
        change_state(STATE_CAVE_TALKING_BEFORE_CLIMB);
    }
}

void process_cave_talking_before_climb() {
    // Hugo talks before climbing (~4 seconds based on sync timing)
    if (get_state_time() > 4.0) {
        change_state(STATE_CAVE_CLIMBING);
    }
}

void process_cave_intro() {
    if (get_state_time() > 2.5) {
        change_state(STATE_CAVE_CLIMBING);
    }
}

void process_cave_climbing() {
    if (get_state_time() > 5.1) {
        change_state(STATE_CAVE_WAITING_INPUT);
    }
}

bool process_cave_waiting_input(SDL_Event* event) {
    if (event && event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_3) {
            game_ctx.cave_selected_rope = 0;
            change_state(STATE_CAVE_GOING_ROPE);
            return true;
        } else if (event->key.keysym.sym == SDLK_6) {
            game_ctx.cave_selected_rope = 1;
            change_state(STATE_CAVE_GOING_ROPE);
            return true;
        } else if (event->key.keysym.sym == SDLK_9) {
            game_ctx.cave_selected_rope = 2;
            change_state(STATE_CAVE_GOING_ROPE);
            return true;
        }
    }
    return false;
}

void process_cave_going_rope() {
    double durations[] = {3.2, 4.0, 4.9};
    
    if (get_state_time() > durations[game_ctx.cave_selected_rope]) {
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

void process_cave_lost() {
    // Hugo animation based on selected rope
    double durations[] = {4.5, 4.5, 4.5};
    if (get_state_time() > durations[game_ctx.cave_selected_rope]) {
        change_state(STATE_CAVE_LOST_SPRING);
    }
}

void process_cave_lost_spring() {
    if (get_state_time() > 3.9) {
        change_state(STATE_END);
    }
}

void process_cave_scylla_lost() {
    // Scylla loses animation (Hugo wins)
    double durations[] = {6.3, 5.6, 4.3}; // bird, leaves, ropes
    
    if (get_state_time() > durations[game_ctx.cave_win_type]) {
        if (game_ctx.cave_win_type == 2) {
            // Only ropes path goes through spring
            change_state(STATE_CAVE_SCYLLA_SPRING);
        } else {
            change_state(STATE_CAVE_FAMILY_CAGE_OPENS);
        }
    }
}

void process_cave_scylla_spring() {
    // Scylla spring animation
    if (get_state_time() > 3.5) {
        change_state(STATE_CAVE_FAMILY_CAGE_OPENS);
    }
}

void process_cave_family_cage_opens() {
    // Family cage opens
    if (get_state_time() > 3.0) {
        change_state(STATE_CAVE_FAMILY_HAPPY);
    }
}

void process_cave_family_happy() {
    if (get_state_time() > 5.0) {
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
    
    while (!quit && state_info.current_state != STATE_END) {
        // Handle events
        bool event_processed = false;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (state_info.current_state == STATE_INSTRUCTIONS) {
                    event_processed = process_instructions(&event);
                } else if (state_info.current_state == STATE_FOREST_PLAYING) {
                    process_playing(&event);
                    event_processed = true;
                } else if (state_info.current_state == STATE_CAVE_WAITING_INPUT) {
                    event_processed = process_cave_waiting_input(&event);
                }
            }
        }
        
        // Process current state
        switch (state_info.current_state) {
            case STATE_INSTRUCTIONS:
                if (!event_processed) {
                    render_instructions();
                }
                break;
                
            // Forest states
            case STATE_FOREST_WAIT_INTRO:
                process_wait_intro();
                render_wait_intro();
                break;
            case STATE_FOREST_PLAYING:
                if (!event_processed) {
                    process_playing(NULL);
                }
                render_playing();
                break;
            case STATE_FOREST_BRANCH_ANIMATION:
                process_forest_branch_animation();
                render_forest_branch_animation();
                break;
            case STATE_FOREST_BRANCH_TALKING:
                process_forest_branch_talking();
                render_forest_branch_talking();
                break;
            case STATE_FOREST_FLYING_START:
                process_forest_flying_start();
                render_forest_flying_start();
                break;
            case STATE_FOREST_FLYING_TALKING:
                process_forest_flying_talking();
                render_forest_flying_talking();
                break;
            case STATE_FOREST_FLYING_FALLING:
                process_forest_flying_falling();
                render_forest_flying_falling();
                break;
            case STATE_FOREST_FLYING_FALLING_HANG_ANIMATION:
                process_forest_flying_falling_hang_animation();
                render_forest_flying_falling_hang_animation();
                break;
            case STATE_FOREST_FLYING_FALLING_HANG_TALKING:
                process_forest_flying_falling_hang_talking();
                render_forest_flying_falling_hang_talking();
                break;
            case STATE_FOREST_ROCK_ANIMATION:
                process_forest_rock_animation();
                render_forest_rock_animation();
                break;
            case STATE_FOREST_ROCK_HIT_ANIMATION:
                process_forest_rock_hit_animation();
                render_forest_rock_hit_animation();
                break;
            case STATE_FOREST_ROCK_TALKING:
                process_forest_rock_talking();
                render_forest_rock_talking();
                break;
            case STATE_FOREST_TRAP_ANIMATION:
                process_forest_trap_animation();
                render_forest_trap_animation();
                break;
            case STATE_FOREST_TRAP_TALKING:
                process_forest_trap_talking();
                render_forest_trap_talking();
                break;
            case STATE_FOREST_SCYLLA_BUTTON:
                process_forest_scylla_button();
                render_forest_scylla_button();
                break;
            case STATE_FOREST_TALKING_AFTER_HURT:
                process_forest_talking_after_hurt();
                render_forest_talking_after_hurt();
                break;
            case STATE_FOREST_TALKING_GAME_OVER:
                process_forest_talking_game_over();
                render_forest_talking_game_over();
                break;
            case STATE_FOREST_WIN_TALKING:
                process_forest_win_talking();
                render_forest_win_talking();
                break;
                
            // Cave states
            case STATE_CAVE_WAITING_BEFORE_TALKING:
                process_cave_waiting_before_talking();
                render_cave_waiting_before_talking();
                break;
            case STATE_CAVE_TALKING_BEFORE_CLIMB:
                process_cave_talking_before_climb();
                render_cave_talking_before_climb();
                break;
            case STATE_CAVE_CLIMBING:
                process_cave_climbing();
                render_cave_climbing();
                break;
            case STATE_CAVE_WAITING_INPUT:
                if (!event_processed) {
                    render_cave_waiting_input();
                }
                break;
            case STATE_CAVE_GOING_ROPE:
                process_cave_going_rope();
                render_cave_going_rope();
                break;
            case STATE_CAVE_LOST:
                process_cave_lost();
                render_cave_lost();
                break;
            case STATE_CAVE_LOST_SPRING:
                process_cave_lost_spring();
                render_cave_lost_spring();
                break;
            case STATE_CAVE_SCYLLA_LOST:
                process_cave_scylla_lost();
                render_cave_scylla_lost();
                break;
            case STATE_CAVE_SCYLLA_SPRING:
                process_cave_scylla_spring();
                render_cave_scylla_spring();
                break;
            case STATE_CAVE_FAMILY_CAGE_OPENS:
                process_cave_family_cage_opens();
                render_cave_family_cage_opens();
                break;
            case STATE_CAVE_FAMILY_HAPPY:
                process_cave_family_happy();
                render_cave_family_happy();
                break;
                
            case STATE_END:
                quit = true;
                break;
        }
        
        state_info.frame_count++;
        SDL_Delay(1000 / 30);
    }
    
    printf("Game ended. Final score: %d\n", game_ctx.score);
}

// Cleanup
void cleanup() {
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
    // Require passing the data dir as argument
    if (argc < 2) {
        printf("Usage: %s <data_directory>\n", argv[0]);
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
