# Hugo - into the multiverse

Requires Python 3 and ffmpeg installed in the system. In windows, it requires `ffmpeg.exe` or similar to be present in this directory, it can be the ["essentials" version](https://www.gyan.dev/ffmpeg/builds/)

## Usage:
- [Game assets can be downloaded from here (gold version)](https://sites.google.com/view/ajugarconhugopc/juegos?authuser=0)
- Download scoreboard resource from here: [Part 1](https://i.imgur.com/X2kpAqX.png) and [Part 2](https://i.imgur.com/VQnPYWx.png), store them in `resources/scores` folder, with the names "sprite1.png" and "sprite2.png" respectively.
- Install requirements using `pip install -r requirements.txt`

## Audios
Game audio and music is not mandatory, if you want to enable it you need to run a few extra things:

- Download mp3 from here: https://www.youtube.com/watch?v=maJVRkefBhw and place in `resources/music/a_party_for_me.mp3`
- Run [Audio Server](../audio-server) if you want to play the audios from the game.
- Execute `game.py <path to resource folder>`

## Key mapping:

Exit: F12

### Player 1 (blue)
Phone off hook: F1
Phone hung up: F2
```
123
qwe
asd
z
```

### Player 2 (green)
Phone off hook: F3
Phone hung up: F4
```
456
rty
fgh
x
```

### Player 3 (red)
Phone off hook: F5
Phone hung up: F6

```
789
uio
jkl
c
```

### Player 4 (white)
Phone off hook: F7
Phone hung up: F8
Keys: numeric keypad
