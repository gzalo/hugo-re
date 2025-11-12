# Hugo Lite - Single Player C/SDL2 Version

A simplified, single-player version of the Hugo forest+cave game written in C using SDL2.

## Features

- Single-player gameplay
- Simple keyboard controls
- Similar game logic as the Python version
- Basic sound support (SDL_mixer)
- Lightweight implementation in C

## Requirements

- SDL2
- SDL2_image
- SDL2_mixer
- GCC or compatible C compiler
- Make (for building)

### Installing dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev
```

**macOS (with Homebrew):**
```bash
brew install sdl2 sdl2_image sdl2_mixer
```

**Windows:**
Install MINGW and Download SDL2 development libraries from https://www.libsdl.org/

## Building

```bash
make
```

## Running

```bash
make run
# or
./hugo_lite
```

## Controls

**Forest Game:**
- **2 / UP Arrow**: Jump over obstacles and collect sacks
- **8 / DOWN Arrow**: Duck under trees
- **5**: Start game from instructions screen

**Cave Bonus Game:**
- **3, 6, 9**: Choose a rope when prompted

**General:**
- **ESC**: Quit the game
