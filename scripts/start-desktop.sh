#!/bin/sh
# Start each Hugo component in its own terminal after the desktop session opens.
set -eu

if ! command -v lxterminal >/dev/null 2>&1; then
    echo "lxterminal is required (install it with: sudo apt install lxterminal)" >&2
    exit 1
fi

scripts_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

for script in \
    0-audio-server.sh \
    1-phone0.sh \
    2-phone1.sh \
    3-phone2.sh \
    4-phone3.sh \
    5-launch-hugo.sh
do
    lxterminal --no-remote --title="${script%.sh}" \
        --command="/bin/bash \"$scripts_dir/$script\"" &
    sleep 1
done
