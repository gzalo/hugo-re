#!/bin/sh
sudo chmod +0666 /dev/uinput

cd /home/gzalo/hugo-re/audio-server/
python audio_server.py \
        --host 0.0.0.0 \
        --assets /home/gzalo/BigFileConverted \
        --sinks vCable1_Out,vCable2_Out,vCable3_Out,vCable4_Out

read -p "Press any key to continue" x
