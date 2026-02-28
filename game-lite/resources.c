#include "common.h"
#include "decoder.h"

// ── Helpers ───────────────────────────────────────────────────────────────────

Texture* animation_get_sync_frame(Animation animation, int frame_index) {
    if (animation.sync_data == NULL) {
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

Texture* animation_get_frame(Animation animation, int frame_index) {
    if (frame_index >= animation.frame_count) {
        printf("Animation index out of range!\n");
        frame_index = animation.frame_count - 1;
    }
    return animation.frames[frame_index];
}

// ── Surface cache ─────────────────────────────────────────────────────────────

#define MAX_CACHE_ENTRIES 64

typedef struct {
    char path[512];
    SDL_Surface** surfaces;
    int count;
} SurfaceCacheEntry;

static SurfaceCacheEntry surface_cache[MAX_CACHE_ENTRIES];
static int surface_cache_size = 0;

// Make every pure-black (R=0,G=0,B=0) pixel fully transparent.
// Matches the post-load step in forest_resources.py for hillsday.cgf.
static void make_black_transparent(SDL_Surface* surf) {
    if (!surf) return;
    SDL_LockSurface(surf);
    uint8_t* pixels = (uint8_t*)surf->pixels;
    for (int y = 0; y < surf->h; y++) {
        uint8_t* row = pixels + y * surf->pitch;
        for (int x = 0; x < surf->w; x++) {
            uint8_t* p = row + x * 4; // [R, G, B, A] for RGBA32
            if (p[0] == 0 && p[1] == 0 && p[2] == 0) p[3] = 0;
        }
    }
    SDL_UnlockSurface(surf);
}

// Return cached surfaces for path, decoding and caching on first access.
static SDL_Surface** get_surfaces(const char* path, int* out_count) {
    for (int i = 0; i < surface_cache_size; i++) {
        if (strcmp(surface_cache[i].path, path) == 0) {
            *out_count = surface_cache[i].count;
            return surface_cache[i].surfaces;
        }
    }

    if (surface_cache_size >= MAX_CACHE_ENTRIES) {
        printf("Warning: surface cache full, cannot cache %s\n", path);
        *out_count = 0;
        return NULL;
    }

    SDL_Surface** surfaces = NULL;
    int count = decode_surfaces_from_file(path, &surfaces);
    if (count < 0) {
        *out_count = 0;
        return NULL;
    }

    SurfaceCacheEntry* e = &surface_cache[surface_cache_size++];
    strncpy(e->path, path, sizeof(e->path) - 1);
    e->path[sizeof(e->path) - 1] = '\0';
    e->surfaces = surfaces;
    e->count = count;

    *out_count = count;
    return surfaces;
}

static void free_surface_cache(void) {
    for (int i = 0; i < surface_cache_size; i++) {
        for (int j = 0; j < surface_cache[i].count; j++) {
            SDL_FreeSurface(surface_cache[i].surfaces[j]);
        }
        free(surface_cache[i].surfaces);
    }
    surface_cache_size = 0;
}

// ── Loading helpers ───────────────────────────────────────────────────────────

// Get a single texture from frame `frame_index` of a decoded file.
static Texture* tex_from_file(const char* path, int frame_index) {
    int count;
    SDL_Surface** surfaces = get_surfaces(path, &count);
    if (!surfaces || frame_index >= count || !surfaces[frame_index]) {
        printf("Warning: frame %d not available in %s\n", frame_index, path);
        return NULL;
    }
    return texture_from_surface(surfaces[frame_index]);
}

// Fill a Texture* array from frames [start, end] of a decoded file.
static void frames_from_file(Texture** out, const char* path, int start, int end) {
    int count;
    SDL_Surface** surfaces = get_surfaces(path, &count);
    for (int i = start; i <= end; i++) {
        int idx = i - start;
        if (!surfaces || i >= count || !surfaces[i]) {
            out[idx] = NULL;
        } else {
            out[idx] = texture_from_surface(surfaces[i]);
        }
    }
}

// Build an Animation from frames [start, end] of a decoded file.
// If sync_oos_path is non-NULL, load sync data from that .oos binary file.
static Animation anim_from_file(const char* path, int start, int end,
                                const char* sync_oos_path) {
    Animation anim;
    anim.frame_count = end - start + 1;
    anim.sync_data   = NULL;
    anim.sync_count  = 0;
    anim.frames = (Texture**)malloc((size_t)anim.frame_count * sizeof(Texture*));
    if (!anim.frames) { anim.frame_count = 0; return anim; }

    frames_from_file(anim.frames, path, start, end);

    if (sync_oos_path) {
        int* indices = NULL;
        int cnt = decode_oos_from_file(sync_oos_path, &indices);
        if (cnt > 0) {
            anim.sync_data  = indices;
            anim.sync_count = cnt;
        }
    }
    return anim;
}

// Load sync data from a .oos binary file under ForestData/Syncs/.
static int* load_sync_oos(const char* data_dir, const char* filename, int* out_count) {
    char path[512];
    snprintf(path, sizeof(path), "%s/ForestData/Syncs/%s", data_dir, filename);
    int* indices = NULL;
    int cnt = decode_oos_from_file(path, &indices);
    if (cnt < 0) { *out_count = 0; return NULL; }
    *out_count = cnt;
    return indices;
}

// ── init_textures ─────────────────────────────────────────────────────────────

void init_textures(const char* data_dir) {
    memset(&textures, 0, sizeof(GameTextures));

    char path[512];
    char sync_path[512];

    // Fixed assets already in native format
    textures.instruction_screen = load_texture("../game/resources/images/instruction_Forest.png");
    textures.bg_gradient        = load_texture("../game/resources/fixed_assets/gradient.bmp");

    // Arrow buttons — stored as pre-exported PNGs (arrows.cgf_0.png … _3.png)
    for (int i = 0; i < 4; i++) {
        snprintf(path, sizeof(path),
                 "../game/resources/fixed_assets/arrows.cgf_%d.png", i);
        textures.arrows[i] = load_texture(path);
    }

    // Background layers (single-frame CGFs)
    // hillsday: make pure-black pixels transparent (matches forest_resources.py)
    snprintf(path, sizeof(path), "%s/ForestData/gfx/hillsday.cgf", data_dir);
    {
        int count;
        SDL_Surface** surfs = get_surfaces(path, &count);
        if (surfs && count > 0 && surfs[0]) {
            make_black_transparent(surfs[0]);
            textures.bg_hillsday = texture_from_surface(surfs[0]);
        }
    }

    snprintf(path, sizeof(path), "%s/ForestData/gfx/paratrees.cgf",  data_dir);
    textures.bg_trees = tex_from_file(path, 0);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/paraground.cgf", data_dir);
    textures.bg_ground = tex_from_file(path, 0);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/GRASS.cgf",      data_dir);
    textures.grass = tex_from_file(path, 0);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/LEAVES1.cgf",    data_dir);
    textures.leaves1 = tex_from_file(path, 0);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/LEAVES2.cgf",    data_dir);
    textures.leaves2 = tex_from_file(path, 0);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/SCOREBRD.bmp",   data_dir);
    textures.scoreboard = load_texture(path);  // already a BMP

    snprintf(path, sizeof(path), "%s/ForestData/gfx/WALL.cgf",       data_dir);
    textures.end_mountain = tex_from_file(path, 0);

    // Hugo animations (multi-frame CGFs)
    snprintf(path, sizeof(path), "%s/ForestData/gfx/hugoside.cgf",    data_dir);
    frames_from_file(textures.hugo_side, path, 0, 7);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/hugohop.cgf",     data_dir);
    frames_from_file(textures.hugo_jump, path, 0, 2);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/kravle.cgf",      data_dir);
    frames_from_file(textures.hugo_crawl, path, 0, 7);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/hugo_hello.cgf",  data_dir);
    textures.hugo_telllives = anim_from_file(path, 0, 15, NULL);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/hand1.cgf",       data_dir);
    textures.hugo_hand1 = tex_from_file(path, 0);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/hand2.cgf",       data_dir);
    textures.hugo_hand2 = tex_from_file(path, 0);

    // Obstacles
    snprintf(path, sizeof(path), "%s/ForestData/gfx/catapult.cgf",    data_dir);
    frames_from_file(textures.catapult, path, 0, 7);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/faelde.cgf",      data_dir);
    frames_from_file(textures.trap, path, 0, 5);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/stone.cgf",       data_dir);
    frames_from_file(textures.rock, path, 0, 7);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/branch-swing.cgf",data_dir);
    frames_from_file(textures.tree, path, 0, 6);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/saek.cgf",        data_dir);
    frames_from_file(textures.sack, path, 0, 3);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/lonetree.cgf",    data_dir);
    textures.lone_tree = tex_from_file(path, 0);

    // HUD elements
    snprintf(path, sizeof(path), "%s/ForestData/gfx/SCORES.cgf",      data_dir);
    textures.score_numbers = tex_from_file(path, 0);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/HUGOSTAT.cgf",    data_dir);
    textures.hugo_lives = tex_from_file(path, 0);

    // Forest hurt animations (TIL files)
    snprintf(path, sizeof(path), "%s/ForestData/gfx/BRANCH-GROGGY.til", data_dir);
    textures.hugohitlog = anim_from_file(path, 0, 42, NULL);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/BRANCH-SPEAK.til",  data_dir);
    textures.hugohitlog_talk = anim_from_file(path, 0, 17, NULL);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/HGKATFLY.til",      data_dir);
    textures.catapult_fly  = anim_from_file(path,   0, 113, NULL);
    textures.catapult_fall = anim_from_file(path, 115, 189, NULL);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/CATAPULT-SPEAK.til",data_dir);
    textures.catapult_airtalk = anim_from_file(path, 0, 15, NULL);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/HGKATHNG.TIL",      data_dir);
    textures.catapult_hang = anim_from_file(path, 0, 12, NULL);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/hanging_mouth.cgf",  data_dir);
    textures.catapult_hangspeak = anim_from_file(path, 0, 11, NULL);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/hugo-rock.til",      data_dir);
    textures.hugo_lookrock = anim_from_file(path, 0, 14, NULL);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/HGROCK.TIL",        data_dir);
    textures.hit_rock = anim_from_file(path, 0, 60, NULL);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/MSYNCRCK.TIL",      data_dir);
    textures.hit_rock_sync = anim_from_file(path, 0, 17, NULL);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/TRAP-HURTS.til",     data_dir);
    textures.hugo_traphurt = anim_from_file(path, 0, 9, NULL);

    snprintf(path, sizeof(path), "%s/ForestData/gfx/traptalk.til",       data_dir);
    textures.hugo_traptalk = anim_from_file(path, 0, 15, NULL);

    // Forest sync data (binary .oos files)
    textures.sync_start      = load_sync_oos(data_dir, "005-01.oos", &textures.sync_start_count);
    textures.sync_rock       = load_sync_oos(data_dir, "005-02.oos", &textures.sync_rock_count);
    textures.sync_dieonce    = load_sync_oos(data_dir, "005-03.oos", &textures.sync_dieonce_count);
    textures.sync_trap       = load_sync_oos(data_dir, "005-04.oos", &textures.sync_trap_count);
    textures.sync_lastlife   = load_sync_oos(data_dir, "005-05.oos", &textures.sync_lastlife_count);
    textures.sync_catapult_talktop = load_sync_oos(data_dir, "005-08.oos",
                                                   &textures.sync_catapult_talktop_count);
    textures.sync_catapult_hang    = load_sync_oos(data_dir, "005-10.oos",
                                                   &textures.sync_catapult_hang_count);
    textures.sync_hitlog     = load_sync_oos(data_dir, "005-11.oos", &textures.sync_hitlog_count);
    textures.sync_gameover   = load_sync_oos(data_dir, "005-12.oos", &textures.sync_gameover_count);
    textures.sync_levelcompleted = load_sync_oos(data_dir, "005-13.oos",
                                                 &textures.sync_levelcompleted_count);

    // Cave animations (STAIRS.TIL used for both talks and climbs)
    snprintf(path,      sizeof(path),      "%s/RopeOutroData/gfx/STAIRS.TIL",         data_dir);
    snprintf(sync_path, sizeof(sync_path), "%s/RopeOutroData/Syncs/002-06.oos",        data_dir);
    textures.cave_talks  = anim_from_file(path,  0, 12, sync_path);
    textures.cave_climbs = anim_from_file(path, 11, 51, NULL);

    // CASELIVE.TIL — multiple ranges
    snprintf(path, sizeof(path), "%s/RopeOutroData/gfx/CASELIVE.TIL", data_dir);
    textures.cave_first_rope    = anim_from_file(path,   0,  32, NULL);
    textures.cave_second_rope   = anim_from_file(path,  33,  72, NULL);
    textures.cave_third_rope    = anim_from_file(path,  73, 121, NULL);
    textures.cave_scylla_leaves = anim_from_file(path, 122, 177, NULL);
    textures.cave_scylla_bird   = anim_from_file(path, 178, 240, NULL);
    textures.cave_scylla_ropes  = anim_from_file(path, 241, 283, NULL);
    textures.cave_scylla_spring = anim_from_file(path, 284, 318, NULL);
    textures.cave_family_cage   = anim_from_file(path, 319, 352, NULL);

    // CASEDIE.TIL — multiple ranges
    snprintf(path, sizeof(path), "%s/RopeOutroData/gfx/CASEDIE.TIL", data_dir);
    textures.cave_hugo_puff_first  = anim_from_file(path, 122, 166, NULL);
    textures.cave_hugo_puff_second = anim_from_file(path, 167, 211, NULL);
    textures.cave_hugo_puff_third  = anim_from_file(path, 212, 256, NULL);
    textures.cave_hugo_spring      = anim_from_file(path, 257, 295, NULL);

    snprintf(path, sizeof(path), "%s/RopeOutroData/gfx/HAPPY.TIL",   data_dir);
    textures.cave_happy = anim_from_file(path, 0, 111, NULL);

    snprintf(path, sizeof(path), "%s/RopeOutroData/gfx/hugo.cgf",    data_dir);
    textures.cave_hugo_sprite = tex_from_file(path, 0);

    // Cave score font (digits 0–9 are frames of SCORE.cgf)
    snprintf(path, sizeof(path), "%s/RopeOutroData/gfx/SCORE.cgf",   data_dir);
    frames_from_file(textures.cave_score_font, path, 0, 9);

    // Surfaces are no longer needed; textures live in GPU memory
    free_surface_cache();
}

// ── Audio ─────────────────────────────────────────────────────────────────────

static Mix_Chunk* load_audio(const char* path) {
    Mix_Chunk* chunk = Mix_LoadWAV(path);
    if (!chunk) {
        printf("Warning: Could not load audio %s: %s\n", path, Mix_GetError());
        return NULL;
    }
    return chunk;
}

void init_audio(const char* data_dir) {
    memset(&audio, 0, sizeof(GameAudio));

    char path[512];
    int loaded = 0;

#define LOAD_AUDIO(field, fmt, ...) \
    snprintf(path, sizeof(path), fmt, __VA_ARGS__); \
    audio.field = load_audio(path); \
    if (audio.field) loaded++;

    // Forest speech
    LOAD_AUDIO(speak_start,            "%s/ForestData/speaks/005-01.wav", data_dir)
    LOAD_AUDIO(speak_rock,             "%s/ForestData/speaks/005-02.wav", data_dir)
    LOAD_AUDIO(speak_dieonce,          "%s/ForestData/speaks/005-03.wav", data_dir)
    LOAD_AUDIO(speak_trap,             "%s/ForestData/speaks/005-04.wav", data_dir)
    LOAD_AUDIO(speak_lastlife,         "%s/ForestData/speaks/005-05.wav", data_dir)
    LOAD_AUDIO(speak_catapult_up,      "%s/ForestData/speaks/005-06.wav", data_dir)
    LOAD_AUDIO(speak_catapult_hit,     "%s/ForestData/speaks/005-07.wav", data_dir)
    LOAD_AUDIO(speak_catapult_talktop, "%s/ForestData/speaks/005-08.wav", data_dir)
    LOAD_AUDIO(speak_catapult_down,    "%s/ForestData/speaks/005-09.wav", data_dir)
    LOAD_AUDIO(speak_catapult_hang,    "%s/ForestData/speaks/005-10.wav", data_dir)
    LOAD_AUDIO(speak_hitlog,           "%s/ForestData/speaks/005-11.wav", data_dir)
    LOAD_AUDIO(speak_gameover,         "%s/ForestData/speaks/005-12.wav", data_dir)
    LOAD_AUDIO(speak_levelcompleted,   "%s/ForestData/speaks/005-13.wav", data_dir)

    // Forest SFX
    LOAD_AUDIO(sfx_bg_atmosphere,  "%s/ForestData/sfx/atmos-lp.wav",   data_dir)
    LOAD_AUDIO(sfx_lightning_warning, "%s/ForestData/sfx/warning.wav", data_dir)
    LOAD_AUDIO(sfx_hugo_knock,     "%s/ForestData/sfx/knock.wav",      data_dir)
    LOAD_AUDIO(sfx_hugo_hittrap,   "%s/ForestData/sfx/crunch.wav",     data_dir)
    LOAD_AUDIO(sfx_hugo_launch,    "%s/ForestData/sfx/skriid.wav",     data_dir)
    LOAD_AUDIO(sfx_sack_normal,    "%s/ForestData/sfx/sack-norm.wav",  data_dir)
    LOAD_AUDIO(sfx_sack_bonus,     "%s/ForestData/sfx/sack.wav",       data_dir)
    LOAD_AUDIO(sfx_tree_swush,     "%s/ForestData/sfx/wush.wav",       data_dir)
    LOAD_AUDIO(sfx_hugo_hitlog,    "%s/ForestData/sfx/bell.wav",       data_dir)
    LOAD_AUDIO(sfx_catapult_eject, "%s/ForestData/sfx/fjeder.wav",     data_dir)
    LOAD_AUDIO(sfx_birds,          "%s/ForestData/sfx/birds-lp.wav",   data_dir)
    LOAD_AUDIO(sfx_hugo_hitscreen, "%s/ForestData/sfx/hit_screen.wav", data_dir)
    LOAD_AUDIO(sfx_hugo_screenklir,"%s/ForestData/sfx/klirr.wav",      data_dir)
    LOAD_AUDIO(sfx_hugo_crash,     "%s/ForestData/sfx/kineser.wav",    data_dir)
    LOAD_AUDIO(sfx_hugo_hangstart, "%s/ForestData/sfx/knage-start.wav",data_dir)
    LOAD_AUDIO(sfx_hugo_hang,      "%s/ForestData/sfx/knage-lp.wav",   data_dir)

    for (int i = 0; i < 5; i++) {
        snprintf(path, sizeof(path), "%s/ForestData/sfx/fumle%d.wav", data_dir, i);
        audio.sfx_hugo_walk[i] = load_audio(path);
        if (audio.sfx_hugo_walk[i]) loaded++;
    }

    // Cave speech
    LOAD_AUDIO(cave_her_er_vi,        "%s/RopeOutroData/speak/002-05.wav", data_dir)
    LOAD_AUDIO(cave_trappe_snak,      "%s/RopeOutroData/speak/002-06.wav", data_dir)
    LOAD_AUDIO(cave_nu_kommer_jeg,    "%s/RopeOutroData/speak/002-07.wav", data_dir)
    LOAD_AUDIO(cave_afskylia_snak,    "%s/RopeOutroData/speak/002-08.wav", data_dir)
    LOAD_AUDIO(cave_hugo_katapult,    "%s/RopeOutroData/speak/002-09.wav", data_dir)
    LOAD_AUDIO(cave_hugo_skyd_ud,     "%s/RopeOutroData/speak/002-10.wav", data_dir)
    LOAD_AUDIO(cave_afskylia_skyd_ud, "%s/RopeOutroData/speak/002-11.wav", data_dir)
    LOAD_AUDIO(cave_hugoline_tak,     "%s/RopeOutroData/speak/002-12.wav", data_dir)

    // Cave SFX
    LOAD_AUDIO(cave_stemning,      "%s/RopeOutroData/SFX/BA-13.WAV",   data_dir)
    LOAD_AUDIO(cave_fodtrin1,      "%s/RopeOutroData/SFX/BA-15.WAV",   data_dir)
    LOAD_AUDIO(cave_fodtrin2,      "%s/RopeOutroData/SFX/BA-16.WAV",   data_dir)
    LOAD_AUDIO(cave_hiv_i_reb,     "%s/RopeOutroData/SFX/BA-17.WAV",   data_dir)
    LOAD_AUDIO(cave_fjeder,        "%s/RopeOutroData/SFX/BA-18.WAV",   data_dir)
    LOAD_AUDIO(cave_pre_puf,       "%s/RopeOutroData/SFX/BA-21.WAV",   data_dir)
    LOAD_AUDIO(cave_puf,           "%s/RopeOutroData/SFX/BA-22.WAV",   data_dir)
    LOAD_AUDIO(cave_tast_trykket,  "%s/RopeOutroData/SFX/BA-24.WAV",   data_dir)
    LOAD_AUDIO(cave_pre_fanfare,   "%s/RopeOutroData/SFX/BA-101.WAV",  data_dir)
    LOAD_AUDIO(cave_fanfare,       "%s/RopeOutroData/SFX/BA-102.WAV",  data_dir)
    LOAD_AUDIO(cave_fugle_skrig,   "%s/RopeOutroData/SFX/BA-104.WAV",  data_dir)
    LOAD_AUDIO(cave_trappe_grin,   "%s/RopeOutroData/SFX/HEXHAHA.WAV", data_dir)
    LOAD_AUDIO(cave_skrig,         "%s/RopeOutroData/SFX/SKRIG.WAV",   data_dir)
    LOAD_AUDIO(cave_score_counter, "%s/RopeOutroData/SFX/COUNTER.WAV", data_dir)

#undef LOAD_AUDIO

    printf("Loaded %d audio files\n", loaded);
}
