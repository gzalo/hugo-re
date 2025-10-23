# Hugo - into the multiverse

Requires Python 3 and `ffmpeg.exe` or similar, can be the ["essentials" version](https://www.gyan.dev/ffmpeg/builds/)

- [Game can be downloaded from here (gold version)](https://sites.google.com/view/ajugarconhugopc/juegos?authuser=0)
- [Resources need to be converted using this converter](../converter)

## Usage:
- Install requirements using `pip install -r requirements.txt`
- Download mp3 from here: https://www.youtube.com/watch?v=maJVRkefBhw and place in `RopeOutroData/SFX/a_party_for_me.mp3` inside the converted resource folder
- Extract the audios from the videos using the following ffmpeg commands:
```
mkdir -p converted_resources/audio_for_videos/ar
ffmpeg -i resources/videos/ar/attract_demo.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/ar/attract_demo.wav
ffmpeg -i resources/videos/ar/have_luck.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/ar/have_luck.wav
ffmpeg -i resources/videos/ar/hello_hello.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/ar/hello_hello.wav
ffmpeg -i resources/videos/ar/press_5.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/ar/press_5.wav
ffmpeg -i resources/videos/ar/scylla_cave.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/ar/scylla_cave.wav
ffmpeg -i resources/videos/ar/you_lost.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/ar/you_lost.wav

mkdir -p converted_resources/audio_for_videos/cl
ffmpeg -i resources/videos/cl/attract_demo.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/cl/attract_demo.wav
ffmpeg -i resources/videos/cl/have_luck.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/cl/have_luck.wav
ffmpeg -i resources/videos/cl/hello_hello.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/cl/hello_hello.wav
ffmpeg -i resources/videos/cl/press_5.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/cl/press_5.wav
ffmpeg -i resources/videos/cl/scylla_cave.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/cl/scylla_cave.wav
ffmpeg -i resources/videos/cl/you_lost.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/cl/you_lost.wav

mkdir -p converted_resources/audio_for_videos/dn
ffmpeg -i resources/videos/dn/attract_demo.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/dn/attract_demo.wav
ffmpeg -i resources/videos/dn/have_luck.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/dn/have_luck.wav
ffmpeg -i resources/videos/dn/hello_hello.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/dn/hello_hello.wav
ffmpeg -i resources/videos/dn/press_5.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/dn/press_5.wav
ffmpeg -i resources/videos/dn/scylla_cave.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/dn/scylla_cave.wav
ffmpeg -i resources/videos/dn/you_lost.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/dn/you_lost.wav

mkdir -p converted_resources/audio_for_videos/fr
ffmpeg -i resources/videos/fr/attract_demo.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/fr/attract_demo.wav
ffmpeg -i resources/videos/fr/have_luck.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/fr/have_luck.wav
ffmpeg -i resources/videos/fr/hello_hello.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/fr/hello_hello.wav
ffmpeg -i resources/videos/fr/press_5.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/fr/press_5.wav
ffmpeg -i resources/videos/fr/scylla_cave.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/fr/scylla_cave.wav
ffmpeg -i resources/videos/fr/you_lost.avi -vn -acodec pcm_s16le converted_resources/audio_for_videos/fr/you_lost.wav
```
- Execute `game.py <path to converted resource folder>`

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
