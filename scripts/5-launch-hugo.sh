#!/bin/sh

export DISPLAY=:0.0
export FULLSCREEN=1
export SDL_AUDIODRIVER=pulseaudio
cd /home/gzalo/hugo-re/game/
python game.py /home/gzalo/BigFileConverted 2>&1 | tee /home/gzalo/hugo_log.log

