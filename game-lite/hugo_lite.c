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
static GameState current_state;
static StateMetadata state_metadata;

GameState process_instructions(InputState state) {
    if (state.key_start || get_state_time(&state_metadata) > 3) {
        return STATE_FOREST;
    }
    return STATE_NONE;
}

void render_instructions() {
    render(textures.instruction_screen, 0, 0);
}

// Main game loop
void game_loop() {
    bool quit = false;
        
    // Initialize
    current_state = STATE_INSTRUCTIONS;
    reset_state(&state_metadata);
    srand(time(NULL));
    InputState input_state = {0};
    
    while (!quit && current_state != STATE_END) {
        // Handle events

        if(render_getevents(&input_state)){
            quit = true;
            break;
        }

        GameState new_state = STATE_NONE;
        if(current_state == STATE_INSTRUCTIONS){
            new_state = process_instructions(input_state);
        } else if(current_state == STATE_FOREST){
            new_state = process_forest(input_state);
        } else if(current_state == STATE_CAVE){
            new_state = process_cave(input_state);
        }
        
        if(new_state != STATE_NONE) {
            printf("State transition: %d -> %d\n", current_state, new_state);
            
            // Pass forest score to cave when transitioning
            if (current_state == STATE_FOREST && new_state == STATE_CAVE) {
                int forest_score = get_forest_score();
                printf("Passing forest score to cave: %d\n", forest_score);
                set_cave_score(forest_score);
            }
            current_state = new_state;
            reset_state(&state_metadata);
            if(current_state == STATE_FOREST){
                on_enter_forest();
            } else if(current_state == STATE_CAVE){
                on_enter_cave();
            }
        }

        if(current_state == STATE_INSTRUCTIONS){
            render_instructions();
        } else if(current_state == STATE_FOREST) {
            render_forest();
        } else if(current_state == STATE_CAVE) {
            render_cave();
        }
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
