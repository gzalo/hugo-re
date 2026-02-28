#include "decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <dirent.h>

// palette[i] = {r, g, b, a}
typedef uint8_t Palette[256][4];

// ── Helpers ──────────────────────────────────────────────────────────────────

static int32_t read_i32(const uint8_t* data, size_t offset) {
    return (int32_t)((uint32_t)data[offset]
        | ((uint32_t)data[offset + 1] << 8)
        | ((uint32_t)data[offset + 2] << 16)
        | ((uint32_t)data[offset + 3] << 24));
}

static int16_t read_i16(const uint8_t* data, size_t offset) {
    return (int16_t)((uint16_t)data[offset] | ((uint16_t)data[offset + 1] << 8));
}

// Read a whole file into a malloc'd buffer. Caller must free().
static uint8_t* read_file(const char* path, size_t* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (len <= 0) { fclose(f); return NULL; }
    uint8_t* data = (uint8_t*)malloc((size_t)len);
    if (!data) { fclose(f); return NULL; }
    if (fread(data, 1, (size_t)len, f) != (size_t)len) { free(data); fclose(f); return NULL; }
    fclose(f);
    *out_len = (size_t)len;
    return data;
}

// Case-insensitive extension check (e.g. ext=".cgf")
static int ext_matches(const char* name, const char* ext) {
    size_t nlen = strlen(name);
    size_t elen = strlen(ext);
    if (nlen < elen) return 0;
    const char* p = name + nlen - elen;
    for (size_t i = 0; i < elen; i++) {
        if (tolower((unsigned char)p[i]) != tolower((unsigned char)ext[i])) return 0;
    }
    return 1;
}

// ── Palette helpers ───────────────────────────────────────────────────────────

// 256 × 3-byte RGB entries, alpha forced to 255  (used by TIL and LZP headers)
static void palette_from_data(Palette pal, const uint8_t* data, size_t offset) {
    for (int i = 0; i < 256; i++) {
        pal[i][0] = data[offset + 0]; // R
        pal[i][1] = data[offset + 1]; // G
        pal[i][2] = data[offset + 2]; // B
        pal[i][3] = 255;
        offset += 3;
    }
}

// 256 × 4-byte little-endian ARGB entries (Java packed int, A forced to 255)
// In memory: B, G, R, pad  →  R=data[+2], G=data[+1], B=data[+0]
static void palette_from_alpha_data(Palette pal, const uint8_t* data, size_t offset) {
    for (int i = 0; i < 256; i++) {
        pal[i][0] = data[offset + 2]; // R
        pal[i][1] = data[offset + 1]; // G
        pal[i][2] = data[offset + 0]; // B
        pal[i][3] = 255;
        offset += 4;
    }
}

// ── Surface construction ──────────────────────────────────────────────────────

static SDL_Surface* make_surface(const uint8_t* pixels, int width, int height) {
    SDL_Surface* s = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    if (!s) return NULL;
    SDL_LockSurface(s);
    memcpy(s->pixels, pixels, (size_t)width * height * 4);
    SDL_UnlockSurface(s);
    return s;
}

static void set_pixel(uint8_t* pixels, int pos, const Palette pal, uint8_t idx) {
    pixels[pos * 4 + 0] = pal[idx][0];
    pixels[pos * 4 + 1] = pal[idx][1];
    pixels[pos * 4 + 2] = pal[idx][2];
    pixels[pos * 4 + 3] = pal[idx][3];
}

// ── CGF decoder ──────────────────────────────────────────────────────────────

// Port of _cgf_parse_line() from converter.py
static void cgf_parse_line(uint8_t* pixels, const uint8_t* data, int idx,
                            int length, int x, int y, const Palette pal, int width) {
    int i = 0;
    while (i < length) {
        uint8_t v = data[idx + i];
        i++; // consume opcode (mirrors the inner i++ in the Java original)

        if (v == 0) {
            uint8_t off = data[idx + i];
            if (off == 0) return;
            x += off;
        } else if (v == 1) { // UNUSED in original game
            int count = data[idx + i];
            for (int k = 0; k < count; k++) {
                i++;
                uint8_t pal_idx = data[idx + i];
                set_pixel(pixels, x + width * y, pal, pal_idx);
                x++;
                i++; // extra i++ present in case v=1
            }
        } else if (v == 2) { // UNUSED in original game
            int count = data[idx + i];
            i++;
            uint8_t value = data[idx + i];
            i++;
            for (int k = 0; k < count; k++) {
                set_pixel(pixels, x + width * y, pal, value);
                x++;
            }
        } else if (v == 3) {
            int count = data[idx + i];
            for (int k = 0; k < count; k++) {
                i++;
                uint8_t pal_idx = data[idx + i];
                set_pixel(pixels, x + width * y, pal, pal_idx);
                x++;
            }
        } else if (v == 4) {
            int count = data[idx + i];
            i++;
            uint8_t value = data[idx + i];
            for (int k = 0; k < count; k++) {
                set_pixel(pixels, x + width * y, pal, value);
                x++;
            }
        } else {
            printf("Unknown CGF opcode: %d at data[%d]\n", v, idx + i);
        }

        i++; // outer for-loop increment
    }
}

// Search parent directory of file_path for a sibling .til and load its palette.
// Returns 1 on success, 0 if none found.
static int find_sibling_til_palette(const char* file_path, Palette pal) {
    char dir[512];
    strncpy(dir, file_path, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = '\0';

    char* sep = strrchr(dir, '/');
    if (!sep) sep = strrchr(dir, '\\');
    if (sep) {
        *sep = '\0';
    } else {
        dir[0] = '.'; dir[1] = '\0';
    }

    DIR* d = opendir(dir);
    if (!d) return 0;

    struct dirent* entry;
    while ((entry = readdir(d)) != NULL) {
        if (!ext_matches(entry->d_name, ".til")) continue;

        char til_path[1024];
        snprintf(til_path, sizeof(til_path), "%s/%s", dir, entry->d_name);
        size_t til_len;
        uint8_t* til_data = read_file(til_path, &til_len);
        if (til_data && til_len > 0x20 + 256 * 3) {
            palette_from_data(pal, til_data, 0x20);
            free(til_data);
            closedir(d);
            return 1;
        }
        free(til_data);
    }
    closedir(d);
    return 0;
}

static int decode_cgf(const uint8_t* data, size_t len, const char* file_path,
                      SDL_Surface*** out_surfaces) {
    Palette pal;
    if (!find_sibling_til_palette(file_path, pal)) {
        if (len < 0x400) {
            printf("Warning: Cannot find palette for CGF %s\n", file_path);
            return -1;
        }
        palette_from_alpha_data(pal, data, len - 0x400);
    }

    if (len < 12) return -1;
    int32_t total_frames = read_i32(data, 8);
    if (total_frames <= 0 || total_frames > 100000) return -1;

    SDL_Surface** surfaces = (SDL_Surface**)calloc((size_t)total_frames, sizeof(SDL_Surface*));
    if (!surfaces) return -1;

    for (int32_t fi = 0; fi < total_frames; fi++) {
        size_t meta = (size_t)(7 * 4) + (size_t)fi * (size_t)(6 * 4);
        if (meta + 24 > len) break;

        int32_t width  = read_i32(data, meta + 8);
        int32_t height = read_i32(data, meta + 12);
        int32_t payload_offset = read_i32(data, meta + 20);

        if (width < 0 || height < 0) {
            printf("Warning: invalid CGF frame size %dx%d in %s\n", width, height, file_path);
            for (int32_t j = 0; j < fi; j++) SDL_FreeSurface(surfaces[j]);
            free(surfaces);
            return -1;
        }

        if (width == 0 || height == 0) {
            surfaces[fi] = SDL_CreateRGBSurfaceWithFormat(0, 1, 1, 32, SDL_PIXELFORMAT_RGBA32);
            continue;
        }

        uint8_t* pixels = (uint8_t*)calloc((size_t)width * (size_t)height * 4, 1);
        if (!pixels) {
            for (int32_t j = 0; j < fi; j++) SDL_FreeSurface(surfaces[j]);
            free(surfaces);
            return -1;
        }

        size_t pixel_base = (size_t)(7 * 4) + (size_t)total_frames * (size_t)(6 * 4);
        size_t start = pixel_base + (size_t)payload_offset;

        for (int32_t y = 0; y < height; y++) {
            if (start + 4 > len) break;
            int32_t line_len = read_i32(data, start);
            if (line_len >= 4) {
                cgf_parse_line(pixels, data, (int)(start + 4), line_len - 4,
                               0, (int)y, pal, (int)width);
            }
            start += (size_t)(line_len > 0 ? line_len : 4);
        }

        surfaces[fi] = make_surface(pixels, (int)width, (int)height);
        free(pixels);
    }

    *out_surfaces = surfaces;
    return (int)total_frames;
}

// ── TIL decoder ──────────────────────────────────────────────────────────────

static int decode_til(const uint8_t* data, size_t len, SDL_Surface*** out_surfaces) {
    if (len < 0x320) return -1;

    Palette pal;
    palette_from_data(pal, data, 0x20);

    int total_width        = (int16_t)read_i16(data, 8);
    int total_height       = (int16_t)read_i16(data, 10);
    int total_width_tiles  = data[0x15];
    int total_height_tiles = data[0x17];

    if (total_width_tiles == 0 || total_height_tiles == 0) return -1;
    if (total_width <= 0 || total_height <= 0) return -1;

    int tile_width  = total_width  / total_width_tiles;
    int tile_height = total_height / total_height_tiles;
    int total_frames = (int16_t)read_i16(data, 6) + 2;
    int tiles_per_frame = total_width_tiles * total_height_tiles;
    int delta = tiles_per_frame * 2; // 2 bytes per tile_id
    size_t tile_data_offset = (size_t)total_frames * (size_t)delta + 0x320;

    SDL_Surface** surfaces = (SDL_Surface**)calloc((size_t)total_frames, sizeof(SDL_Surface*));
    if (!surfaces) return -1;

    for (int fi = 0; fi < total_frames; fi++) {
        size_t frame_offset = 0x320 + (size_t)fi * (size_t)delta;
        uint8_t* pixels = (uint8_t*)calloc((size_t)total_width * (size_t)total_height * 4, 1);
        if (!pixels) {
            for (int j = 0; j < fi; j++) SDL_FreeSurface(surfaces[j]);
            free(surfaces);
            return -1;
        }

        int valid = 1;
        size_t fo = frame_offset;

        for (int l = 0; l < tiles_per_frame && valid; l++) {
            if (fo + 2 > len) { valid = 0; break; }
            int tile_id = (int16_t)read_i16(data, fo);
            fo += 2;
            int x_off = (l % total_width_tiles) * tile_width;
            int y_off = (l / total_width_tiles) * tile_height;
            size_t curr = tile_data_offset + (size_t)tile_id * (size_t)tile_height * (size_t)tile_width;

            for (int ty = 0; ty < tile_height && valid; ty++) {
                for (int tx = 0; tx < tile_width; tx++) {
                    if (curr >= len) { valid = 0; break; }
                    uint8_t color_idx = data[curr++];
                    int pos = (tx + x_off) + (ty + y_off) * total_width;
                    set_pixel(pixels, pos, pal, color_idx);
                }
            }
        }

        if (!valid) {
            free(pixels);
            // Return however many frames we decoded successfully
            for (int j = fi; j < total_frames; j++) surfaces[j] = NULL;
            *out_surfaces = surfaces;
            return fi;
        }

        surfaces[fi] = make_surface(pixels, total_width, total_height);
        free(pixels);
    }

    *out_surfaces = surfaces;
    return total_frames;
}

// ── LZP decoder ──────────────────────────────────────────────────────────────

static int decode_lzp(const uint8_t* data, size_t len, SDL_Surface*** out_surfaces) {
    if (len < 12 + 0x20 + 256 * 3) return -1;

    Palette pal;
    palette_from_data(pal, data, 0x20);

    int total_frames = read_i32(data, 0);
    int width        = read_i32(data, 4);
    int height       = read_i32(data, 8);

    if (total_frames <= 0 || width <= 0 || height <= 0) return -1;

    const int N = 4095;

    SDL_Surface** surfaces = (SDL_Surface**)calloc((size_t)total_frames, sizeof(SDL_Surface*));
    if (!surfaces) return -1;

    for (int fi = 0; fi < total_frames; fi++) {
        size_t off2off = len - (size_t)total_frames * 4 + (size_t)fi * 4;
        if (off2off + 4 > len) break;
        int32_t offset_local = read_i32(data, off2off);
        if (offset_local < 0 || (size_t)offset_local + 4 > len) break;
        int32_t compressed_len = read_i32(data, (size_t)offset_local);

        uint8_t* pixels = (uint8_t*)calloc((size_t)width * (size_t)height * 4, 1);
        if (!pixels) {
            for (int j = 0; j < fi; j++) SDL_FreeSurface(surfaces[j]);
            free(surfaces);
            return -1;
        }

        size_t data_ptr = (size_t)offset_local + 4;
        size_t data_end = data_ptr + (size_t)compressed_len;

        uint8_t window[4096];
        memset(window, 0, sizeof(window));
        int dst_pos      = 0;
        int window_index = 4078;
        int bit_idx      = 0;
        int end_flag     = 0;
        uint8_t flags    = 0;

        while (!end_flag) {
            if (bit_idx == 0) {
                if (data_ptr >= data_end) break;
                flags = data[data_ptr++];
            }

            if (flags & 1) {
                // Literal
                if (data_ptr >= data_end) break;
                uint8_t c = data[data_ptr++];
                if (dst_pos >= width * height) break;
                set_pixel(pixels, dst_pos, pal, c);
                dst_pos++;
                window[window_index] = c;
                window_index = (window_index + 1) & N;
            } else {
                // Back-reference
                if (data_ptr + 1 >= data_end) break;
                uint8_t b1 = data[data_ptr++];
                uint8_t b2 = data[data_ptr++];
                int back_offset = (int)b1 | (((int)(b2 & 0xF0)) << 4);
                int run_len     = (b2 & 0xF) + 3;
                for (int k = 0; k < run_len; k++) {
                    uint8_t c = window[(back_offset + k) & N];
                    if (dst_pos >= width * height) { end_flag = 1; break; }
                    set_pixel(pixels, dst_pos, pal, c);
                    dst_pos++;
                    window[window_index] = c;
                    window_index = (window_index + 1) & N;
                }
            }

            flags >>= 1;
            bit_idx = (bit_idx + 1) & 7;
        }

        surfaces[fi] = make_surface(pixels, width, height);
        free(pixels);
    }

    *out_surfaces = surfaces;
    return total_frames;
}

// ── OOS decoder ──────────────────────────────────────────────────────────────

int decode_oos_from_file(const char* path, int** out_indices) {
    size_t len;
    uint8_t* data = read_file(path, &len);
    if (!data) {
        printf("Warning: Could not open OOS file %s\n", path);
        *out_indices = NULL;
        return -1;
    }

    if (len < 0x20) { free(data); *out_indices = NULL; return -1; }

    int32_t offset_to_length = read_i32(data, 0x14);
    int32_t offset_to_data   = read_i32(data, 0x18);

    if (offset_to_length < 0 || (size_t)offset_to_length + 4 > len) {
        free(data); *out_indices = NULL; return -1;
    }
    int32_t length = read_i32(data, (size_t)offset_to_length);

    if (length <= 0 || offset_to_data < 0 || (size_t)offset_to_data + (size_t)length > len) {
        free(data); *out_indices = NULL; return -1;
    }

    int* indices = (int*)malloc((size_t)length * sizeof(int));
    if (!indices) { free(data); *out_indices = NULL; return -1; }

    for (int32_t i = 0; i < length; i++) {
        indices[i] = data[(size_t)offset_to_data + (size_t)i];
    }

    free(data);
    *out_indices = indices;
    return (int)length;
}

// ── Dispatch ─────────────────────────────────────────────────────────────────

int decode_surfaces_from_file(const char* path, SDL_Surface*** out_surfaces) {
    size_t len;
    uint8_t* data = read_file(path, &len);
    if (!data) {
        printf("Warning: Could not open file %s\n", path);
        *out_surfaces = NULL;
        return -1;
    }

    int count = -1;
    if (ext_matches(path, ".cgf")) {
        count = decode_cgf(data, len, path, out_surfaces);
    } else if (ext_matches(path, ".til")) {
        count = decode_til(data, len, out_surfaces);
    } else if (ext_matches(path, ".lzp")) {
        count = decode_lzp(data, len, out_surfaces);
    } else {
        printf("Warning: unsupported format for %s\n", path);
        *out_surfaces = NULL;
    }

    free(data);
    return count;
}
