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
#include "resources.h"

GameTextures textures;
GameAudio audio;
GameState current_state;
double current_state_start_time;

GameState process_instructions(InputState state) {
    if (state.key_start || get_state_time(current_state_start_time) > 3) {
        return STATE_FOREST;
    }
    return STATE_NONE;
}

void render_instructions() {
    render(textures.instruction_screen, 0, 0);
}

typedef GameState (*ProcessFunc)(InputState state);
typedef void (*RenderFunc)(void);
typedef void (*OnEnterFunc)(void);

ProcessFunc process_funcs[STATE_END] = {
    NULL,
    process_instructions,
    process_forest,
    process_cave
};

RenderFunc render_funcs[STATE_END] = {
    NULL,
    render_instructions,
    render_forest,
    render_cave
};

OnEnterFunc on_enter_funcs[STATE_END] = {
    NULL,
    NULL,
    on_enter_forest,
    on_enter_cave
};

// Main game loop
void game_loop() {
    bool quit = false;
        
    // Initialize
    current_state = STATE_INSTRUCTIONS;
    current_state_start_time = get_game_time();
    srand(time(NULL));
    InputState input_state = {0};
    
    while (!quit && current_state != STATE_END) {
        // Handle events

        if(render_getevents(&input_state)){
            quit = true;
            break;
        }

        GameState new_state = process_funcs[current_state](input_state);
        
        if(new_state != STATE_NONE) {
            printf("State transition: %d -> %d\n", current_state, new_state);
            current_state = new_state;
            current_state_start_time = get_game_time();
            reset_state_events();
            OnEnterFunc on_enter_func = on_enter_funcs[current_state];
            if (on_enter_func != NULL) {
                on_enter_func();
            }
        }

        render_funcs[current_state]();
        render_step();
    }
    
    printf("Game ended\n");
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
