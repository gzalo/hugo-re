"""
Preprocessing script: converts AVI videos to WebM for browser playback.
Run once from the repo root before building the web version:
    python game-web/scripts/convert_videos.py

Requires ffmpeg.exe to be present in game/ (already in repo).
"""

import os
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(os.path.dirname(SCRIPT_DIR))
# When run from repo root, resolve relative to this script's location
REPO_ROOT = os.path.dirname(SCRIPT_DIR)  # game-web/
REPO_ROOT = os.path.dirname(REPO_ROOT)   # repo root

FFMPEG = os.path.join(REPO_ROOT, "game", "ffmpeg.exe")
INPUT_BASE = os.path.join(REPO_ROOT, "game", "resources", "videos")
OUTPUT_BASE = os.path.join(REPO_ROOT, "game-web", "resources", "videos")

LANGS = ["ar", "cl", "dn", "fr"]


def convert(src: str, dst: str) -> None:
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    if os.path.exists(dst):
        print(f"  skip (exists): {os.path.basename(dst)}")
        return
    cmd = [
        FFMPEG,
        "-i", src,
        "-c:v", "libvpx-vp9",
        "-b:v", "0",
        "-crf", "30",
        "-an",          # no audio (game uses separate audio)
        "-y",
        dst,
    ]
    print(f"  converting: {os.path.basename(src)} -> {os.path.basename(dst)}")
    result = subprocess.run(cmd, capture_output=True)
    if result.returncode != 0:
        print(f"  ERROR: {result.stderr.decode('utf-8', errors='replace')}", file=sys.stderr)


def main() -> None:
    if not os.path.exists(FFMPEG):
        print(f"ffmpeg not found at: {FFMPEG}", file=sys.stderr)
        sys.exit(1)

    for lang in LANGS:
        in_dir = os.path.join(INPUT_BASE, lang)
        out_dir = os.path.join(OUTPUT_BASE, lang)
        if not os.path.isdir(in_dir):
            print(f"Missing input dir: {in_dir}")
            continue
        print(f"[{lang}]")
        for fname in os.listdir(in_dir):
            if fname.lower().endswith(".avi"):
                src = os.path.join(in_dir, fname)
                dst = os.path.join(out_dir, os.path.splitext(fname)[0] + ".webm")
                convert(src, dst)

    print("Done.")


if __name__ == "__main__":
    main()
