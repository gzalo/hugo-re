# OOS format

Header:
- 8 bytes Name (e.g SYNC2000)
- 4 bytes version (48 01 32 06)
- 4 bytes unk
- 4 bytes unk 
- 4 bytes offset to data len
- 4 bytes offset to data

Middle data:
- 1 byte layer name len (X)
- X bytes layer name (e.g LAYER 01)
- null terminated WAV filename (e.g ":L:\Hugo2D\005 Forest\Speak\Argentina.df\8b.22k\005-08.wav")
- null terminated TIL filename (e.g "+L:\!Work\Video\Sync-anims\Hugo\CloseUp.til")

Lip sync data:
- 4 bytes Data len
- N bytes data (one byte per frame), which says which frame of the animation should be shown when playing that audio
