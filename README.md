# hugo-re
Reverse engineering Hugo games

- Use [QuickBMS](http://aluigi.altervista.org/quickbms.htm) with [Hugo3](http://aluigi.altervista.org/bms/hugo3.bms) script to extract 4HUGO.DAT

# Formats found

- PCX or PCC: Picture exchange format. Credits.pcx with a sepia effect are used for showing scores in the TV show.
- RAW or PIC or BLK: [HSI Raw](http://justsolve.archiveteam.org/wiki/HSI_Raw) pictures, can be viewed in XNView. "mhwanh" header. vgapal.raw, pcpal.raw may be used?
- SND: raw signed 8 bit PCM, 1 channel, 22050 Hz. Can be opened with audacity. kult.snd is weird

Unknown:
- DAT/BIN: binary data about games. Very sparse. Variations for TV and Arcade modes that have different difficulty. Seems to be made of blocks of 12 bytes, no headers
    - poleX.bin has a different structure: only 00s at the start, then 10h-16h and 19h

- TIL/TI2/TI4: tiled animations? first 0x20 header, 0x20-0x320 seems pallete, 0x320 on data. See intro.til with ITE logo

- CBR: Image? First 0x20 seems like a header. Seems compressed as related OFS contains offsets beyond EOF

- PBR: Image? First 28 bytes seems like a header. Seems compressed as related OFS contains offsets beyond EOF

- BRS: Animations: N frames, each one starts with a (W H X Y) 16 byte header. Palette is unknown. May be used for speech related sequences. Uncompressed

- OFS/OFF: Usually comes with CBR, PBR and BRS, has N 4 byte offsets into the other file, maybe frames

- BSH: only in pladebag: has some texts: FORM/GRAB/DRNG

