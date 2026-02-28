#ifndef DECODER_H
#define DECODER_H

#include <SDL2/SDL.h>

// Decode a CGF, TIL, or LZP file into an array of SDL_Surface*.
// Each surface must be freed with SDL_FreeSurface; the array itself with free().
// Returns the number of surfaces on success, or -1 on error.
int decode_surfaces_from_file(const char* path, SDL_Surface*** out_surfaces);

// Decode an OOS sync file into an array of raw byte frame indices.
// The array must be freed with free().
// Returns the number of indices on success, or -1 on error.
int decode_oos_from_file(const char* path, int** out_indices);

#endif
